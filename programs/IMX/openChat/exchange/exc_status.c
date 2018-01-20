#include "libmini.h"

#include "exc_comm_def.h"
#include "exc_gid_map.h"
#include "exc_uid_map.h"
#include "exc_message_dispatch.h"
#include "exc_status.h"
#include "comm_print.h"

static void each_gid_fcb(char gid[MAX_GID_SIZE], size_t idx, void *usr)
{
	char *cid = usr;
	exc_cidmap_rem_gid(cid, gid);
	exc_gidmap_rem_cid(gid, cid);
}

int erase_client(int fd)
{
	char cid[MAX_CID_SIZE] = {};
	snprintf(cid, sizeof(cid), "%d", fd);

	char    uid[MAX_UID_SIZE] = {};
	exc_cidmap_get_uid(cid, uid);

	exc_uidmap_del_cid(uid);
	exc_cidmap_del_uid(cid);

	exc_cidmap_get_gid(cid, each_gid_fcb, cid);
	return 0;
}

void send_status_msg(int clientfd, int status)
{
	char cid[MAX_CID_SIZE] = {};
	get_cid(cid, clientfd);

	struct comm_message msg = {};
	commmsg_make(&msg, DEFAULT_MSG_SIZE);
	commmsg_sets(&msg, g_serv_info.login_gateway_fd, 0, PUSH_METHOD);

	/*[status] | [closed/connected] | [cid]*/
	commmsg_frame_set(&msg, 0, strlen(cid), cid);
	msg.package.frames_of_package[0] = msg.package.frames;
	msg.package.packages = 1;
	x_printf(D, "send status:%d", status);

	if (status == STEP_INIT) {
		commmsg_frame_set(&msg, 0, 9, "connected");
		msg.package.frames_of_package[0] = msg.package.frames;
		msg.package.packages = 1;
	} else if (status == STEP_STOP) {
		commmsg_frame_set(&msg, 0, 6, "closed");
		msg.package.frames_of_package[0] = msg.package.frames;
		msg.package.packages = 1;
	}

	commmsg_frame_set(&msg, 0, 6, "status");
	msg.package.frames_of_package[0] = msg.package.frames;
	msg.package.packages = 1;

#define debug 1
#ifdef debug
	x_printf(D, "-------------------------------");
	commmsg_print(&msg);
	x_printf(D, "-------------------------------");
#endif

	commapi_send(g_serv_info.commctx, &msg);
	commmsg_free(&msg);
}

