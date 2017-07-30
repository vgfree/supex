#include "comm_message_operator.h"
#include "gid_map.h"
#include "message_dispatch.h"
#include "status.h"
#include "uid_map.h"
#include "libmini.h"
#include "comm_def.h"


int erase_client(int fd)
{
	char    uid[MAX_UID_SIZE] = {};
	int     size = 0;

	find_uid(uid, &size, fd);
	remove_fd(uid);
	remove_uid(fd);

	char *gid_list[MAX_ONE_CID_HAVE_GID] = {};
	size = MAX_ONE_CID_HAVE_GID;
	if (find_gid_list(fd, gid_list, &size) == 0) {
		remove_gid_list(fd, gid_list, size);

		for (int i = 0; i < size; i++) {
			remove_fd_list(gid_list[i], &fd, 1);
			free(gid_list[i]);
		}
	} else {
		x_printf(E, "no fd:%d map , is impossible?", fd);
	}

	return size;
}

void send_status_msg(int clientfd, int status)
{
	char cid[MAX_CID_SIZE] = {};
	get_cid(cid, clientfd);

	struct comm_message msg = {};
	init_msg(&msg);
	set_msg_frame(0, &msg, strlen(cid), cid);
	x_printf(D, "send status:%d", status);

	if (status == STEP_INIT) {
		set_msg_frame(0, &msg, 9, "connected");
	} else if (status == STEP_STOP) {
		set_msg_frame(0, &msg, 6, "closed");
	}

	set_msg_frame(0, &msg, 6, "status");

	set_msg_fd(&msg, g_serv_info.login_server_fd);
	commapi_send(g_serv_info.commctx, &msg);
	destroy_msg(&msg);
}

