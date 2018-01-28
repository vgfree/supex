#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "libmini.h"
#include "comm_api.h"
#include "../manage_skts.h"
#include "stream_i_wrap.h"
#include "stream_o_wrap.h"

extern struct manage_skts g_stream_i_skts;
static bool do_msg_for_each_fcb(int sockfd, int idx, void *usr)
{
	struct comm_message *msg = usr;

	commmsg_sets(msg, sockfd, 0, PUSH_METHOD);

	if (stream_i_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}

	return true;
}

static int downstream_msg(struct comm_message *msg)
{
	int cnt = manage_skts_travel(&g_stream_i_skts, do_msg_for_each_fcb, NULL, msg);

	x_printf(D, "core exchange node travel count:%d.", cnt);
	return 0;
}

static void *_pull_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};

		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		stream_o_wrap_recv(&msg);
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

extern struct manage_skts g_stream_o_skts;
static bool do_msg_for_best_fcb(int sockfd, int idx, void *usr)
{
	struct comm_message *msg = usr;

	commmsg_sets(msg, sockfd, 0, PUSH_METHOD);

	if (stream_o_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
		return true;
	}

	return false;
}

static int upstream_msg(struct comm_message *msg)
{
	int cnt = manage_skts_travel(&g_stream_o_skts, do_msg_for_best_fcb, NULL, msg);

	x_printf(D, "core exchange node travel count:%d.", cnt);
	return 0;
}

static void *_push_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};

		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		stream_i_wrap_recv(&msg);
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

static pthread_t        tid1;
static pthread_t        tid2;
void stream_gateway_work(void)
{
	assert(stream_i_wrap_init() == 0);
	assert(stream_o_wrap_init() == 0);

	/*work pull*/
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	/*work push*/
	err = pthread_create(&tid2, NULL, _push_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	x_printf(I, "stream gateway work!\n");
}

void stream_gateway_wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
	pthread_join(tid2, status);
}

void stream_gateway_stop(void)
{
	x_printf(W, "stream gateway stop!\n");
	stream_i_wrap_exit();
	stream_o_wrap_exit();
}

