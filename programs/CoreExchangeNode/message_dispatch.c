#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "comm_message_operator.h"
#include "fd_manager.h"
#include "gid_map.h"
#include "message_dispatch.h"
#include "status.h"
#include "uid_map.h"
#include "libmini.h"

struct server_info g_serv_info = {};

void get_cid(char cid[MAX_CID_SIZE], const int fd)
{
	struct fd_descriptor des = {};
	fdman_array_at_fd(fd, &des);
	snprintf(cid, MAX_CID_SIZE, "%s|%s:%d", des.uuid, g_serv_info.host, fd);
}

void find_best_gateway(int *fd)
{
	if (g_serv_info.message_gateway_fd > 0) {
		*fd = g_serv_info.message_gateway_fd;
	} else {
		struct fd_node node;

		if (fdman_list_front(MESSAGE_GATEWAY, &node) == -1) {
			x_printf(E, "no gateway server.");
			return;
		}

		*fd = node.fd;
	}
}

static void _handle_cid_message(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(2, msg, &fsz);
	
	char    cid[MAX_CID_SIZE] = {};
	strncpy(cid, frame, fsz);
	x_printf(D, "cid:%s.", cid);

	char *uuid = strtok(cid, "|");//TODO check uuid
	char *host = strtok(NULL, ":");
	if (strcmp(host, g_serv_info.host) == 0) {
		char    *cfd = strtok(NULL, ":");
		int     fd = atoi(cfd);
		x_printf(D, "fd:%d.", fd);
		
		set_msg_fd(msg, fd);
		
		remove_first_nframe(3, msg);
		comm_send(g_serv_info.commctx, msg, true, -1);
	}
}

static int _handle_gid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);

	char    gid[MAX_GID_SIZE] = {};
	memcpy(gid, frame, fsz);
	
	int     fd_list[MAX_ONE_GID_HAVE_CID] = {};
	int	size = MAX_ONE_GID_HAVE_CID;
	int ok = find_fd_list(gid, fd_list, &size);
	if (ok != 0) {
		return 0;
	}

	remove_first_nframe(3, msg);
	x_printf(D, "get_max_msg_frame:%d, fd size:%d", get_max_msg_frame(msg), size);

	for (int i = 0; i < size; i++) {
		x_printf(D, "sent msg to fd:%d.", fd_list[i]);
		set_msg_fd(msg, fd_list[i]);
		comm_send(g_serv_info.commctx, msg, true, -1);
	}

	return 0;
}

static int _handle_uid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	assert(fsz >= MAX_UID_SIZE);

	char    uid[MAX_UID_SIZE] = {};
	memcpy(uid, frame, fsz);
	int fd = find_fd(uid);

	if (fd != -1) {
		remove_first_nframe(3, msg);
		set_msg_fd(msg, fd);
		comm_send(g_serv_info.commctx, msg, true, -1);
	}

	return 0;
}

static int _handle_uid_map(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[MAX_CID_SIZE] = {};

	strncpy(cid, frame, fsz);
	char *uuid = strtok(cid, "|");//TODO check
	char *host = strtok(NULL, ":");

	if (strcmp(host, g_serv_info.host) != 0) {
		x_printf(D, "this cid is not belong to this server");
		return -1;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
	char    *uid = get_msg_frame(3, msg, &fsz);
	assert(fsz >= MAX_UID_SIZE);

	char    uid_buf[MAX_UID_SIZE] = {};
	memcpy(uid_buf, uid, fsz);
	insert_fd(uid_buf, fd);
	return 0;
}

static int _handle_gid_map(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[MAX_CID_SIZE] = {};

	strncpy(cid, frame, fsz);
	char *uuid = strtok(cid, "|");
	char *host = strtok(NULL, ":");

	if (strcmp(host, g_serv_info.host) != 0) {
		x_printf(D, "this host:%s is not belong to this server:%s.", host, g_serv_info.host);
		return -1;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
#if 0
	// 删除与此cid 相关的所有群组关系.
	char    *gid_list[MAX_ONE_CID_HAVE_GID] = {};
	int     size = MAX_ONE_CID_HAVE_GID;

	if (find_gid_list(fd, gid_list, &size) == 0) {
		remove_gid_list(fd, gid_list, size);

		for (int i = 0; i < size; i++) {
			remove_fd_list(gid_list[i], &fd, 1);
			free(gid_list[i]);
		}
	} else {
		x_printf(D, "first insert gidmap. fd:%d.", fd);
	}
#endif

	char    *gid_frame = get_msg_frame(3, msg, &fsz);
	char    gid[MAX_GID_SIZE] = {};
	int     gid_index = 0;

	for (int i = 0; i < fsz; i++) {
		if (gid_frame[i] == ',') {
			insert_fd_list(gid, &fd, 1);
			insert_gid_list(fd, gid);
			memset(gid, 0, MAX_GID_SIZE);
			gid_index = 0;
		} else {
			gid[gid_index++] = gid_frame[i];
		}
	}

	x_printf(D, "insert gid:%s, fd:%d.", gid, fd);
	insert_fd_list(gid, &fd, 1);
	insert_gid_list(fd, gid);
	return 0;
}

static void _downstream_msg(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(1, msg, &fsz);

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

static void _erased_client(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[MAX_CID_SIZE] = {};

	strncpy(cid, frame, fsz);
	char *uuid = strtok(cid, "|");//TODO check
	char *host = strtok(NULL, ":");

	if (strcmp(host, g_serv_info.host) != 0) {
		x_printf(E, "erase host:%s, serv host:%s.", host, g_serv_info.host);
		return;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
	
	struct fd_descriptor des = {};
	fdman_array_remove_fd(fd, &des);

	erase_client(fd);
	x_printf(D, "errase client fd:%d.", fd);

	send_status_msg(fd, FD_CLOSE);
	
	comm_close(g_serv_info.commctx, fd);
}

static void _handle_status(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(3, msg, &fsz);

	if (!frame) {
		x_printf(E, "wrong frame, frame is NULL or not equal closed.");
		return;
	} else if (memcmp(frame, "closed", 6) == 0) {
		_erased_client(msg);
	}
}

static void _classified_message(struct comm_message *msg)
{
	int     frame_size;
	char    *frame = get_msg_frame(0, msg, &frame_size);
	if (!frame) {
		x_printf(E, "wrong frame, and frame is NULL.");
	}

	char    frame_buf[100] = {};
	memcpy(frame_buf, frame, frame_size);
	x_printf(D, "max msg:%d, frame:%s", get_max_msg_frame(msg), frame_buf);

	if (memcmp(frame, "downstream", 10) == 0) {
		_downstream_msg(msg);
	} else {
		x_printf(E, "wrong first frame, frame_size:%d.", frame_size);
	}
}

static void _setting_map(struct comm_message *msg)
{
	x_printf(D, "max msg:%d.", get_max_msg_frame(msg));
	int     frame_size;
	char    *frame = get_msg_frame(0, msg, &frame_size);
	if (!frame) {
		x_printf(E, "wrong frame, and frame is NULL.");
	}

	if (memcmp(frame, "setting", 7) == 0) {
		char *cmd = get_msg_frame(1, msg, &frame_size);

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

static int _verified(struct comm_message *msg)
{
#define debug 1
#ifdef debug
	x_printf(D, "client msg frame number:%d", get_max_msg_frame(msg));
	
	int i = 0;
	for (; i < get_max_msg_frame(msg); i++) {
		int     frame_size = 0;
		char    *frame = get_msg_frame(i, msg, &frame_size);
		char    *buf = malloc(sizeof(char) * (frame_size + 1));
		memcpy(buf, frame, frame_size);
		buf[frame_size] = '\0';
		
		x_printf(D, "%d frame, data:%s", i, buf);
		free(buf);
	}
	x_printf(I, "msg type:%d.", msg->socket_type);
#endif

	if (msg->socket_type == PAIR_METHOD) {
		remove_first_nframe(1, msg);
		char buf[21] = {};
		buf[0] = 0x01;
		set_msg_frame(0, msg, 21, buf);

		x_printf(D, "send message msg type:%d, dsize:%d frame_size:%d frames_of_package:%d frames:%d packages:%d", msg->socket_type, msg->package.dsize, msg->package.frame_size[0], msg->package.frames_of_package[0], msg->package.frames, msg->package.packages);
		comm_send(g_serv_info.commctx, msg, true, -1);
		return 1;
	}

	return 0;
}

void message_dispatch(void)
{
	struct comm_message msg = {};
	init_msg(&msg);
	
	x_printf(D, "comm_recv wait.");
	comm_recv(g_serv_info.commctx, &msg, true, -1);
	
	struct fd_descriptor des;
	fdman_array_at_fd(msg.fd, &des);
	x_printf(D, "des.obj:%d", des.obj);

	if (des.obj == CLIENT) {
		if (_verified(&msg) == 0) {
			int fd = 0;
			find_best_gateway(&fd);

			if (fd > 0) {
				char cid[MAX_CID_SIZE] = {};
				get_cid(cid, msg.fd);
				
				set_msg_frame(0, &msg, strlen(cid), cid);
				set_msg_frame(0, &msg, 8, "upstream");
				set_msg_fd(&msg, fd);
				comm_send(g_serv_info.commctx, &msg, true, -1);
			}
		}
	} else if (des.obj == MESSAGE_GATEWAY) {
		_classified_message(&msg);
	} else if (des.obj == SETTING_SERVER) {
		_setting_map(&msg);
	}

	destroy_msg(&msg);
}

