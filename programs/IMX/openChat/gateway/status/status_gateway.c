#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "libmini.h"
#include "comm_api.h"
#include "../manage_skts.h"
#include "status_i_wrap.h"
#include "status_o_wrap.h"

extern struct manage_skts g_status_o_skts;

static bool do_msg_for_best_fcb(int sockfd, int idx, void *usr)
{
	struct comm_message *msg = usr;

	commmsg_sets(msg, sockfd, 0, PUSH_METHOD);

	if (status_o_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
		return true;
	}

	return false;
}

static int upstream_msg(struct comm_message *msg)
{
	int cnt = manage_skts_travel(&g_status_o_skts, do_msg_for_best_fcb, NULL, msg);

	x_printf(D, "core exchange node travel count:%d.", cnt);
#if 1
	int api_sfd = status_o_get_api_sfd();
	commmsg_sets(msg, api_sfd, 0, PUSH_METHOD);

	if (status_o_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}
#endif
	return 0;
}

static void *_push_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};

		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		status_i_wrap_recv(&msg);
#define debug 1
#ifdef debug
		x_printf(D, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		commmsg_print(&msg);
		x_printf(D, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
#endif

		upstream_msg(&msg);

		commmsg_free(&msg);
	}

	return NULL;
}

static pthread_t tid1;
void status_gateway_work(void)
{
	assert(status_i_wrap_init() == 0);
	assert(status_o_wrap_init() == 0);

	/*work push*/
	int err = pthread_create(&tid1, NULL, _push_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	x_printf(I, "status gateway work!\n");
}

void status_gateway_wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
}

void status_gateway_stop(void)
{
	x_printf(W, "status gateway stop!\n");
	status_i_wrap_exit();
	status_o_wrap_exit();
}

