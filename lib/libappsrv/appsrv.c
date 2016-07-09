#include "appsrv.h"
#include "libmini.h"

#include <assert.h>
#include <string.h>
#include "appsrv_handle.h"
#include "config_reader.h"

static void *g_ctx = NULL;
static AO_SpinLockT *g_send_lock = NULL;
static AO_SpinLockT *g_recv_lock = NULL;

void create_io(int types)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	g_send_lock = (AO_SpinLockT*)malloc(sizeof(AO_SpinLockT));
	g_recv_lock = (AO_SpinLockT*)malloc(sizeof(AO_SpinLockT));
	AO_SpinLockInit(g_send_lock, false);
	AO_SpinLockInit(g_recv_lock, false);
	init_connect(g_ctx, types);
}

void destroy_io(void)
{
	assert(g_ctx);
	destroy_connect();
	zmq_ctx_destroy(g_ctx);
}


int app_recv_msg(enum askt_type type, struct app_msg *msg)
{
	assert(msg);
	AO_SpinLock(g_recv_lock);
	int rc = recv_msg(type, msg);
	AO_SpinUnlock(g_recv_lock);
	return rc;
}


int app_send_msg(enum askt_type type, struct app_msg *msg)
{
	assert(msg && msg->vector_size > 0);
	AO_SpinLock(g_recv_lock);
	int rc = send_msg(type, msg);
	AO_SpinUnlock(g_recv_lock);
	return rc;
}

int app_recv_more_msg(int types, struct app_msg *msg, int flag)
{
	assert(msg);
	AO_SpinLock(g_recv_lock);
	int rc = recv_more_msg(types, msg, flag);
	AO_SpinUnlock(g_recv_lock);
	return rc;
}
