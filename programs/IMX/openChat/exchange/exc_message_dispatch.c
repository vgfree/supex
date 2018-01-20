#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libmini.h"
#include "exc_comm_def.h"
#include "exc_gid_map.h"
#include "exc_uid_map.h"
#include "exc_cid_map.h"
#include "exc_sockfd_manager.h"
#include "exc_message_dispatch.h"
#include "exc_status.h"
#include "comm_print.h"

struct server_info g_serv_info = {};

void get_cid(char cid[MAX_CID_SIZE], const int fd)
{
	struct fd_info des = {};
	fdman_slot_get(fd, &des);
	snprintf(cid, MAX_CID_SIZE, "%s|%s:%d-%d", des.uuid, g_serv_info.host, g_serv_info.port, fd);
}

/*---------------------------------------------------------------------*/
static void find_best_gateway(int *fd)
{
	if (g_serv_info.message_gateway_fd > 0) {
		*fd = g_serv_info.message_gateway_fd;
	} else {
		struct fd_node node;

		if (fdman_list_top(MESSAGE_ROUTER, &node) == -1) {
			x_printf(E, "no gateway server.");
			return;
		}

		*fd = node.fd;
	}
}

static int _dispatch_client(struct comm_message *msg)
{
	/*心跳*/
	if (msg->ptype == PAIR_METHOD) {
		commmsg_frame_del(msg, 0, 1);
		msg->package.frames_of_package[0]--;

		char buf[21] = {};
		buf[0] = 0x01;
		commmsg_frame_set(msg, 0, 21, buf);
		msg->package.frames_of_package[0] = msg->package.frames;
		msg->package.packages = 1;

		x_printf(D, "send message msg type:%d, dsize:%d frame_size:%d frames_of_package:%d frames:%d packages:%d",
				msg->ptype, msg->package.raw_data.len, msg->package.frame_size[0],
				msg->package.frames_of_package[0], msg->package.frames, msg->package.packages);
		commapi_send(g_serv_info.commctx, msg);
		return 1;
	}

	/*上传*/
	int fd = 0;
	find_best_gateway(&fd);
	if (fd > 0) {
		char cid[MAX_CID_SIZE] = {};
		get_cid(cid, msg->fd);

		/*[upstream] | [cid] | ...*/
		commmsg_frame_set(msg, 0, strlen(cid), cid);
		msg->package.frames_of_package[0] = msg->package.frames;
		msg->package.packages = 1;
		commmsg_frame_set(msg, 0, 8, "upstream");
		msg->package.frames_of_package[0] = msg->package.frames;
		msg->package.packages = 1;

		int flags = 0;
		int ptype = 0;
		commmsg_gets(msg, NULL, &flags, &ptype);
		commmsg_sets(msg, fd, flags, ptype);
		commapi_send(g_serv_info.commctx, msg);
		x_printf(D, "upstream!");
	}
	return 0;
}
/*---------------------------------------------------------------------*/
static void _handle_cid_message(struct comm_message *msg)
{
	int     fsz;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_CID_SIZE);
	
	char    cid[MAX_CID_SIZE] = {};
	memcpy(cid, frame, fsz);
	//x_printf(D, "cid:%s.", cid);

	char *uuid = strtok(cid, "|");//TODO check uuid
	char *host = strtok(NULL, ":");
	char *port = strtok(NULL, "-");
	char *cfd = strtok(NULL, "");
	if ((strcmp(host, g_serv_info.host) == 0)
			&& (g_serv_info.port == atoi(port))) {
		int     fd = atoi(cfd);
		x_printf(D, "fd:%d.", fd);

		int flags = 0;
		int ptype = 0;
		commmsg_gets(msg, NULL, &flags, &ptype);
		commmsg_sets(msg, fd, flags, ptype);
		
		commmsg_frame_del(msg, 0, 3);
		msg->package.frames_of_package[0] -= 3;
		commapi_send(g_serv_info.commctx, msg);
	}
}

static int _handle_uid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_UID_SIZE);

	char    uid[MAX_UID_SIZE] = {};
	memcpy(uid, frame, fsz);
	x_printf(D, "uid:%s.", uid);

	char cid[MAX_CID_SIZE] = {};
	int ok = exc_uidmap_get_cid(uid, cid);

	int cfd = atoi(cid);
	if (cfd != -1) {
		commmsg_frame_del(msg, 0, 3);
		msg->package.frames_of_package[0] -= 3;

		int flags = 0;
		int ptype = 0;
		commmsg_gets(msg, NULL, &flags, &ptype);
		commmsg_sets(msg, cfd, flags, ptype);
		commapi_send(g_serv_info.commctx, msg);
	}

	return 0;
}

static void each_cid_fcb(char cid[MAX_CID_SIZE], size_t idx, void *usr)
{
	struct comm_message *msg = usr;
	int cfd = atoi(cid);
	x_printf(D, "sent msg to cfd:%d.", cfd);

	int flags = 0;
	int ptype = 0;
	commmsg_gets(msg, NULL, &flags, &ptype);
	commmsg_sets(msg, cfd, flags, ptype);
	commapi_send(g_serv_info.commctx, msg);
}

static int _handle_gid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_GID_SIZE);

	char    gid[MAX_GID_SIZE] = {};
	memcpy(gid, frame, fsz);
	
	commmsg_frame_del(msg, 0, 3);
	msg->package.frames_of_package[0] -= 3;
	x_printf(D, "commmsg_frame_count:%d", commmsg_frame_count(msg));

	exc_gidmap_get_cid(gid, each_cid_fcb, msg);
	return 0;
}

static void _downstream_msg(struct comm_message *msg)
{
	int     fsz;
	char    *frame = commmsg_frame_get(msg, 1, &fsz);

	if (!frame) {
		x_printf(E, "wrong frame, frame is NULL.");
		return;
	}

	if (fsz != 3) {
		x_printf(E, "downstream 2nd frame size:%d is not equal 3.", fsz);
	}

	if (memcmp(frame, "cid", 3) == 0) {
		_handle_cid_message(msg);
	} else if (memcmp(frame, "uid", 3) == 0) {
		_handle_uid_message(msg);
	} else if (memcmp(frame, "gid", 3) == 0) {
		_handle_gid_message(msg);
	} else {
		x_printf(E, "wrong frame.");
	}
}

static void _dispatch_message(struct comm_message *msg)
{
	int     frame_size;
	char    *frame = commmsg_frame_get(msg, 0, &frame_size);
	if (!frame) {
		x_printf(E, "wrong frame, and frame is NULL.");
	} else {
		//x_printf(D, "max msg:%d", commmsg_frame_count(msg));
	}

	if (memcmp(frame, "downstream", 10) == 0) {
		_downstream_msg(msg);
	} else {
		x_printf(E, "wrong first frame, frame_size:%d.", frame_size);
	}
}

/*---------------------------------------------------------------------*/

static void _erased_client(struct comm_message *msg)
{
	int     fsz;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_CID_SIZE);

	char    cid[MAX_CID_SIZE] = {};
	strncpy(cid, frame, fsz);

	char *uuid = strtok(cid, "|");//TODO check uuid
	char *host = strtok(NULL, ":");
	char *port = strtok(NULL, "-");
	char *cfd = strtok(NULL, "");
	if ((strcmp(host, g_serv_info.host) != 0)
			|| (g_serv_info.port != atoi(port))) {
		x_printf(E, "erase %s:%s, serv %s:%d.", host, port, g_serv_info.host, g_serv_info.port);
		return;
	}

	int     fd = atoi(cfd);

	fdman_slot_del(fd);
	erase_client(fd);/*TODO:和close回调死循环?*/
	x_printf(D, "errase client fd:%d.", fd);

	//send_status_msg(fd, STEP_STOP);

	commapi_close(g_serv_info.commctx, fd);
}

static void _handle_status(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = commmsg_frame_get(msg, 3, &fsz);

	if (!frame) {
		x_printf(E, "wrong frame, frame is NULL or not equal closed.");
		return;
	} else if (memcmp(frame, "closed", 6) == 0) {
		_erased_client(msg);
	}
}

static int _handle_uid_map(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_CID_SIZE);

	char    cid[MAX_CID_SIZE] = {};
	strncpy(cid, frame, fsz);

	char *uuid = strtok(cid, "|");//TODO check uuid
	char *host = strtok(NULL, ":");
	char *port = strtok(NULL, "-");
	char *cfd = strtok(NULL, "");
	if ((strcmp(host, g_serv_info.host) != 0)
			|| (g_serv_info.port != atoi(port))) {
		x_printf(D, "this cid is not belong to this server");
		return -1;
	}

	frame = commmsg_frame_get(msg, 3, &fsz);
	assert(fsz < MAX_UID_SIZE);

	char    uid[MAX_UID_SIZE] = {};
	memcpy(uid, frame, fsz);

	int     fd = atoi(cfd);
	snprintf(cid, sizeof(cid), "%s", fd);
	exc_uidmap_set_cid(uid, cid);
	exc_cidmap_set_uid(cid, uid);
	return 0;
}

static int _handle_gid_map(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = commmsg_frame_get(msg, 2, &fsz);
	assert(fsz < MAX_CID_SIZE);

	char    cid[MAX_CID_SIZE] = {};
	strncpy(cid, frame, fsz);

	char *uuid = strtok(cid, "|");//TODO check uuid
	char *host = strtok(NULL, ":");
	char *port = strtok(NULL, "-");
	char *cfd = strtok(NULL, "");
	if ((strcmp(host, g_serv_info.host) != 0)
			|| (g_serv_info.port != atoi(port))) {
		x_printf(D, "this host:%s is not belong to this server:%s.", host, g_serv_info.host);
		return -1;
	}

	int     fd = atoi(cfd);
	snprintf(cid, sizeof(cid), "%d", fd);

	int     gid_index = 0;
	char    gid[MAX_GID_SIZE] = {};
	char    *gid_frame = commmsg_frame_get(msg, 3, &fsz);
	for (int i = 0; i < fsz; i++) {
		if (gid_frame[i] == ',') {
			exc_gidmap_add_cid(gid, cid);
			exc_cidmap_add_gid(cid, gid);
			memset(gid, 0, MAX_GID_SIZE);
			gid_index = 0;
		} else {
			gid[gid_index++] = gid_frame[i];
		}
	}

	x_printf(D, "insert gid:%s, fd:%d.", gid, fd);
	exc_gidmap_add_cid(gid, cid);
	exc_cidmap_add_gid(cid, gid);
	return 0;
}

static void _dispatch_control(struct comm_message *msg)
{
	x_printf(D, "max msg:%d.", commmsg_frame_count(msg));
	int     frame_size;
	char    *frame = commmsg_frame_get(msg, 0, &frame_size);
	if (!frame) {
		x_printf(E, "wrong frame, and frame is NULL.");
	}

	if (memcmp(frame, "setting", 7) == 0) {
		char *cmd = commmsg_frame_get(msg, 1, &frame_size);

		if (memcmp(cmd, "status", 6) == 0) {
			_handle_status(msg);
		} else if (memcmp(cmd, "uidmap", 6) == 0) {
			_handle_uid_map(msg);
		} else if (memcmp(cmd, "gidmap", 6) == 0) {
			_handle_gid_map(msg);
		}
	} else {
		x_printf(E, "wrong first frame, frame_size:%d.", frame_size);
	}
}

/*---------------------------------------------------------------------*/

void exc_message_dispatch(void)
{
	struct comm_message msg = {};
	commmsg_make(&msg, DEFAULT_MSG_SIZE);
	
	//x_printf(D, "comm_recv wait.");
	commapi_recv(g_serv_info.commctx, &msg);

	struct fd_info des;
	fdman_slot_get(msg.fd, &des);
	x_printf(D, "des type:%d", des.type);
#define debug 1
#ifdef debug
	x_printf(D, "-------------------------------");
	commmsg_print(&msg);
	x_printf(D, "-------------------------------");
#endif

	if (des.type == CLIENT_ROUTER) {
		_dispatch_client(&msg);
	} else if (des.type == MESSAGE_ROUTER) {
		_dispatch_message(&msg);
	} else if (des.type == CONTROL_ROUTER) {
		_dispatch_control(&msg);
	}

	commmsg_free(&msg);
}

