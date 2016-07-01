#include "appsrv.h"
#include "libmini.h"
#include "recv_wraper.h"
#include "send_wraper.h"

#include <assert.h>
#include <string.h>

static void *g_ctx = NULL;
static AO_SpinLockT *g_send_lock = NULL;
static AO_SpinLockT *g_recv_lock = NULL;
void create_io()
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	g_send_lock = (AO_SpinLockT*)malloc(sizeof(AO_SpinLockT));
	g_recv_lock = (AO_SpinLockT*)malloc(sizeof(AO_SpinLockT));
	AO_SpinLockInit(g_send_lock, false);
	AO_SpinLockInit(g_recv_lock, false);
	init_send(g_ctx);
	init_recv(g_ctx);
}

void destroy_io()
{
	assert(g_ctx);
	destroy_send();
	destroy_recv();
	zmq_ctx_destroy(g_ctx);
}

int send_app_msg(struct app_msg *msg)
{
	AO_SpinLock(g_send_lock);
	assert(msg && msg->vector_size > 0);
	int rc = -1;
//	printf("iov_len:%u\n", msg->vector[0].iov_len);

	if ((msg->vector[0].iov_len == 7) &&
		(memcmp("setting", msg->vector[0].iov_base, 7) == 0)) {
		printf("send to _api.\n");
		rc = send_to_api(msg);
	} else {
		rc = send_to_gateway(msg);
	}
	AO_SpinUnlock(g_send_lock);
	return rc;
}

int recv_app_msg(struct app_msg *msg, int *more, int flag)
{
	AO_SpinLock(g_recv_lock);
	assert(msg);
	int rc = recv_all_msg(msg, more, flag);
	AO_SpinUnlock(g_recv_lock);
	return rc;
}

