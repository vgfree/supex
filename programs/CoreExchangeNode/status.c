#include "comm_message_operator.h"
#include "gid_map.h"
#include "message_dispatch.h"
#include "status.h"
#include "uid_map.h"

int erase_client(int fd)
{
	char    uid[30] = {};
	int     size = 0;

	find_uid(uid, &size, fd);
	remove_fd(uid);
	remove_uid(fd);
	char *gid_list[20] = {};//TODO

	if (find_gid_list(fd, gid_list, &size) > 0) {
		remove_gid_list(fd, gid_list, size);

		for (int i = 0; i < size; i++) {
			remove_fd_list(gid_list[i], &fd, 1);
			free(gid_list[i]);
		}
	} else {
		error("no fd:%d map , is impossible?", fd);
	}

	return size;
}

void send_status_msg(int clientfd, int status)
{
	char cid[30] = {};
	strcpy(cid, g_serv_info.ip);
	strcat(cid, ":");
	char buf[10] = {};
	snprintf(buf, 10, "%d", clientfd);
	strcat(cid, buf);
	
	struct comm_message msg = {};
	init_msg(&msg);
	set_msg_frame(0, &msg, strlen(cid), cid);
	log("send status:%d", status);

	if (status == FD_INIT) {
		set_msg_frame(0, &msg, 9, "connected");
	} else if (status == FD_CLOSE) {
		set_msg_frame(0, &msg, 6, "closed");
	}

	set_msg_frame(0, &msg, 6, "status");
	
	set_msg_fd(&msg, g_serv_info.login_server_fd);
	comm_send(g_serv_info.commctx, &msg, true, -1);
	destroy_msg(&msg);
}

