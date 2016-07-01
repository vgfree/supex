#include "comm_message_operator.h"
#include "fd_manager.h"
#include "gid_map.h"
#include "loger.h"
#include "message_dispatch.h"
#include "router.h"
#include "status.h"
#include "uid_map.h"

#ifdef _HKEY_
#include "hkey.h"
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct server_info g_serv_info = {};

void find_best_gateway(int *fd)
{
	if (g_serv_info.message_gateway_fd > 0) {
		*fd = g_serv_info.message_gateway_fd;
	} else {
		struct fd_node node;

		if (list_front(MESSAGE_GATEWAY, &node) == FAILED) {
			error("no gateway server.");
			return;
		}

		*fd = node.fd;
	}
}

static void _handle_cid_message(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[30] = {};

	strncpy(cid, frame, fsz);
	char *ip = strtok(cid, ":");
	log("cid:%s.", cid);

	if (strcmp(ip, g_serv_info.ip) == 0) {
		char    *cfd = strtok(NULL, ":");
		int     fd = atoi(cfd);
		set_msg_fd(msg, fd);
		remove_first_nframe(3, msg);
		log("fd:%d.", fd);
		comm_send(g_serv_info.commctx, msg, true, -1);
	}
}

static int _handle_gid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    gid[20] = {};

	memcpy(gid, frame, fsz);
	int     fd_list[GROUP_SIZE] = {};
	int     size = find_fd_list(gid, fd_list);
	remove_first_nframe(3, msg);
	log("get_max_msg_frame:%d, fd size:%d", get_max_msg_frame(msg), size);

	for (int i = 0; i < size; i++) {
		log("sent msg to fd:%d.", fd_list[i]);
		set_msg_fd(msg, fd_list[i]);
		comm_send(g_serv_info.commctx, msg, true, -1);
	}

	return 0;
}

static int _handle_uid_message(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    uid[20] = {};

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
	char    cid[30] = {};

	strncpy(cid, frame, fsz);
	char *ip = strtok(cid, ":");

	if (strcmp(ip, g_serv_info.ip) != 0) {
		log("this cid is not belong to this server");
		return -1;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
	char    *uid = get_msg_frame(3, msg, &fsz);
	char    uid_buf[20] = {};
	memcpy(uid_buf, uid, fsz);
	insert_fd(uid_buf, fd);
	return 0;
}

static int _handle_gid_map(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[30] = {};

	strncpy(cid, frame, fsz);
	char *ip = strtok(cid, ":");

	if (strcmp(ip, g_serv_info.ip) != 0) {
		log("this ip:%s is not belong to this server:%s.", ip, g_serv_info.ip);
		return -1;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
	// 删除与此cid 相关的所有群组关系.
	char    *gid_list[30] = {};
	int     size = 0;

	if (find_gid_list(fd, gid_list, &size) > 0) {
		remove_gid_list(fd, gid_list, size);

		for (int i = 0; i < size; i++) {
			remove_fd_list(gid_list[i], &fd, 1);
			free(gid_list[i]);
		}
	} else {
		log("first insert gidmap. fd:%d.", fd);
	}

	char    *gid_frame = get_msg_frame(3, msg, &fsz);
	char    gid[20] = {};
	int     gid_index = 0;

	for (int i = 0; i < fsz; i++) {
		if (gid_frame[i] == ',') {
			insert_fd_list(gid, &fd, 1);
			insert_gid_list(fd, gid);
			memset(gid, 0, 20);
			gid_index = 0;
		} else {
			gid[gid_index++] = gid_frame[i];
		}
	}

	log("insert gid:%s, fd:%d.", gid, fd);
	insert_fd_list(gid, &fd, 1);
	insert_gid_list(fd, gid);
	return 0;
}

static void _downstream_msg(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(1, msg, &fsz);

	if (!frame) {
		error("wrong frame, frame is NULL.");
		return;
	}

	if (fsz != 3) {
		error("downstream 2nd frame size:%d is not equal 3.", fsz);
	}

	if (memcmp(frame, "cid", 3) == 0) {
		_handle_cid_message(msg);
	} else if (memcmp(frame, "uid", 3) == 0) {
		_handle_uid_message(msg);
	} else if (memcmp(frame, "gid", 3) == 0) {
		_handle_gid_message(msg);
	} else {
		error("wrong frame.");
	}
}

static void _erased_client(struct comm_message *msg)
{
	int     fsz;
	char    *frame = get_msg_frame(2, msg, &fsz);
	char    cid[30] = {};

	strncpy(cid, frame, fsz);
	char *ip = strtok(cid, ":");

	if (strcmp(ip, g_serv_info.ip) != 0) {
		error("erase ip:%s, serv ip:%s.", ip, g_serv_info.ip);
		return;
	}

	char    *cfd = strtok(NULL, ":");
	int     fd = atoi(cfd);
	comm_close(g_serv_info.commctx, fd);
	erase_client(fd);
	log("errase fd:%d.", fd);
	send_status_msg(fd, FD_CLOSE);
	array_remove_fd(fd);
}

static void _handle_status(struct comm_message *msg)
{
	int     fsz = 0;
	char    *frame = get_msg_frame(3, msg, &fsz);

	if (!frame) {
		error("wrong frame, frame is NULL or not equal closed.");
		return;
	} else if (memcmp(frame, "closed", 6) == 0) {
		_erased_client(msg);
	}
}

static void _classified_message(struct comm_message *msg)
{
	int     frame_size;
	char    *frame = get_msg_frame(0, msg, &frame_size);
	char    frame_buf[100] = {};

	memcpy(frame_buf, frame, frame_size);
	log("max msg:%d, frame:%s", get_max_msg_frame(msg), frame_buf);

	if (!frame) {
		error("wrong frame, and frame is NULL.");
	}

	if (memcmp(frame, "downstream", 10) == 0) {
		_downstream_msg(msg);
	} else {
		error("wrong first frame, frame_size:%d.", frame_size);
	}
}

static void _setting_map(struct comm_message *msg)
{
	log("max msg:%d.", get_max_msg_frame(msg));
	int     frame_size;
	char    *frame = get_msg_frame(0, msg, &frame_size);

	if (!frame) {
		error("wrong frame, and frame is NULL.");
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
		error("wrong first frame, frame_size:%d.", frame_size);
	}
}

static int _verified(struct comm_message *msg)
{
#ifdef _HKEY_
	if (msg->socket_type == PAIR_METHOD) {
		int frame_size = 0;
		char    *frame = get_msg_frame(0, msg, &frame_size);
		char *key = (char *)malloc((frame_size + 1)* sizeof(char));
		memcpy(key, frame, frame_size);
		key[frame_size] = '\0';
		int fd = hkey_get_fd(key);
		char buf[10] = {};
		if (fd == -1) { // first hkey.
			snprintf(buf, 10, "%d", 0);
			set_msg_frame(0, msg, strlen(buf), buf);
		}
		else {
			struct residue_package package;
			char *value = hkey_get_value(key);
			int offset = hkey_get_offset(key);
			init_residue_package(&package, msg->fd, offset, value, strlen(value));
			push_residue_package(&package);
			snprintf(buf, 10, "%d", offset);
			set_msg_frame(0, msg, strlen(buf), buf); // 接收大小。
			hkey_del_fd(key);
			hkey_del_fd_key(fd);
			hkey_del_value(key);
			destroy_residue_package(&package);
		}
		hkey_insert_fd(key, msg->fd);
		comm_send(g_serv_info.commctx, msg, true, -1);
		return 1;
	}
#endif
	return 0;
}

void message_dispatch()
{
	struct comm_message msg = {};

	init_msg(&msg);
	log("comm_recv wait.");
	comm_recv(g_serv_info.commctx, &msg, true, -1);
	struct fd_descriptor des;
	array_at_fd(msg.fd, &des);
	log("des.obj:%d", des.obj);

	if (des.obj == CLIENT) {
		if (_verified(&msg) == 0) {
			int fd = 0;
			find_best_gateway(&fd);

			if (fd > 0) {
				char cid[30] = {};
				strcpy(cid, g_serv_info.ip);
				strcat(cid, ":");
				char buf[10] = {};
				snprintf(buf, 10, "%d", msg.fd);
				strcat(cid, buf);
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

