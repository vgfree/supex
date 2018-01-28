#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "libmini.h"
#include "comm_utils.h"
#include "comm_api.h"
#include "../manage_skts.h"
#include "manage_i_wrap.h"
#include "manage_o_wrap.h"

extern struct manage_skts g_manage_i_skts;
static bool do_msg_for_each_fcb(int sockfd, int idx, void *usr)
{
	struct comm_message *msg = usr;

	commmsg_sets(msg, sockfd, 0, PUSH_METHOD);

	if (manage_i_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}

	return true;
}

static int downstream_msg(struct comm_message *msg)
{
	int cnt = manage_skts_travel(&g_manage_i_skts, do_msg_for_each_fcb, NULL, msg);

	x_printf(D, "core exchange node travel count:%d.", cnt);
	return 0;
}

static void *_pull_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};
		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		manage_o_wrap_recv(&msg);

#define debug 1
#ifdef debug
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
		commmsg_print(&msg);
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
#endif
		downstream_msg(&msg);

		commmsg_free(&msg);
	}

	return NULL;
}

static pthread_t tid1;
void manage_gateway_work(void)
{
	assert(manage_i_wrap_init() == 0);
	assert(manage_o_wrap_init() == 0);

	/*work push*/
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	x_printf(I, "manage gateway work!\n");
}

void manage_gateway_wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
}

void manage_gateway_stop(void)
{
	x_printf(W, "manage gateway stop!\n");
	manage_i_wrap_exit();
	manage_o_wrap_exit();
}

