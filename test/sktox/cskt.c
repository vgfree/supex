#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

#include "skt.h"
#include <zlist.h>
#include <zpoller.h>
#include <zproxy_v2.h>

/*
 * 1.直接调用的资源必须在同一个进程内使用.
 * 2.跨进程通信时，上下文必须重新创建.
 */

static char     *g_own = "my";
static zctx_t   *g_ctx = NULL;

void skt_register(char *name)
{
	if (name) {
		g_own = name;
	}

	assert(!g_ctx);
	g_ctx = zctx_new();
}

struct _zproxy_t
{
	zctx_t  *ctx;			//  Context, allows inproc capture
	void    *pipe;			//  Pipe used by API thread
	void    *frontend;		//  Frontend socket for proxy switching
	void    *backend;		//  Backend socket for proxy switching
};

void *zmq_process_start(void *(*fcb)(void *args), void *args)
{
	pid_t pid = 0;

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(-1);
	} else if (pid == 0) {
		g_ctx = zctx_new();	/*must*/
		fcb(args);
		exit(0);
	} else {
		printf("FORK ONE PROCESS -->PID :%d\n", pid);
	}

	return NULL;
}

struct _zpoller_t
{
	zlist_t         *reader_list;	//  List of sockets to read from
	zmq_pollitem_t  *poll_set;	//  Current zmq_poll set
	void            **poll_readers;	//  Matching table of socket readers
	size_t          poll_size;	//  Size of poll set
	bool            need_rebuild;	//  Does pollset needs rebuilding?
	bool            expired;	//  Did poll timer expire?
	bool            terminated;	//  Did poll call end with EINTR?
};

static zpoller_t *zpoller_new2(struct skt_kernel *rlist, ...)
{
	zpoller_t *self = (zpoller_t *)zmalloc(sizeof(zpoller_t));

	if (self) {
		self->reader_list = zlist_new();

		if (self->reader_list) {
			self->need_rebuild = true;

			struct skt_kernel *head = rlist;

			while (rlist) {
				if (zlist_append(self->reader_list, rlist->skt) == -1) {
					zpoller_destroy(&self);
					break;
				}

				rlist = rlist->next;

				if (rlist == head) {
					break;
				}
			}

			va_list args;
			va_start(args, rlist);
			void *reader = va_arg(args, void *);

			while (reader) {
				if (zlist_append(self->reader_list, reader) == -1) {
					zpoller_destroy(&self);
					break;
				}

				reader = va_arg(args, void *);
			}

			va_end(args);
		}
	}

	return self;
}

static void skt_proxy(void *args, zctx_t *ctx, void *command_pipe)
{
	//  Confirm to API that we've started up
	zsocket_signal(command_pipe);
	zproxy_t *self = (zproxy_t *)args;
	//  Capture socket, if not NULL, receives all data
	void *capture = NULL;

	//  Create poller to work on all three sockets
	struct skt_kernel       *rlist = (struct skt_kernel *)self->frontend;
	struct skt_kernel       *wlist = (struct skt_kernel *)self->backend;
	zpoller_t               *poller = zpoller_new2(rlist, command_pipe, NULL);
	assert(poller);

	bool                    stopped = false;
	struct skt_kernel       *wtemp = wlist;

	while (!stopped) {
		//  Wait for activity on any polled socket, and read incoming message
		void            *which = zpoller_wait(poller, -1);
		zmq_msg_t       msg;
		zmq_msg_init(&msg);
		int send_flags;		//  Flags for outgoing message

		if (which && (zmq_recvmsg(which, &msg, 0) != -1)) {
			send_flags = zsocket_rcvmore(which) ? ZMQ_SNDMORE : 0;

			if (which != command_pipe) {
				void *output = wtemp->skt;
				wtemp = wtemp->next ? wtemp->next : wlist;

				//  Loop on all waiting messages, since polling adds a
				//  non-trivial cost per message, especially on OS/X
				while (true) {
					if (capture) {
						zmq_msg_t dup;
						zmq_msg_init(&dup);
						zmq_msg_copy(&dup, &msg);

						if (zmq_sendmsg(capture, &dup, send_flags) == -1) {
							zmq_msg_close(&dup);
						}
					}

					if (zmq_sendmsg(output, &msg, send_flags) == -1) {
						zmq_msg_close(&msg);
						break;
					}

					if (zmq_recvmsg(which, &msg, ZMQ_DONTWAIT) == -1) {
						break;		//  Presumably EAGAIN
					}

					send_flags = zsocket_rcvmore(which) ? ZMQ_SNDMORE : 0;
				}
			} else {
				char command [10] = { 0 };
				assert(zmq_msg_size(&msg) < 10);
				memcpy(command, zmq_msg_data(&msg), zmq_msg_size(&msg));

				//  Execute API command
				if (streq(command, "PAUSE")) {
					zpoller_destroy(&poller);
					poller = zpoller_new2(command_pipe, NULL);
					assert(poller);
				} else if (streq(command, "RESUME")) {
					zpoller_destroy(&poller);
					rlist = (struct skt_kernel *)self->frontend;
					poller = zpoller_new2(rlist, command_pipe, NULL);
					assert(poller);
				} else if (streq(command, "CAPTURE")) {
					//  Capture flow is always PUSH-to-PULL
					capture = zsocket_new(self->ctx, ZMQ_PUSH);
					char *endpoint = zstr_recv(command_pipe);
					assert(endpoint);

					if (capture) {
						int rc = zsocket_connect(capture, "%s", endpoint);
						assert(rc == 0);
					}

					zstr_free(&endpoint);
				} else if (streq(command, "STOP")) {
					stopped = true;
				} else {
					assert(0);					//  Cannot happen, so die
				}

				//  Signal to caller that we processed the command
				zsocket_signal(command_pipe);
			}
		} else {
			break;			//  Interrupted
		}
	}

	zpoller_destroy(&poller);
}

zproxy_t *zproxy_new2(zctx_t *ctx, struct skt_kernel *rlist, struct skt_kernel *wlist)
{
	assert(ctx);
	zproxy_t *self = (zproxy_t *)zmalloc(sizeof(zproxy_t));

	if (self) {
		self->ctx = ctx;
		self->frontend = rlist;
		self->backend = wlist;
		self->pipe = zthread_fork(ctx, skt_proxy, self);

		if (self->pipe) {
			zsocket_wait(self->pipe);
		} else {
			//  If we ran out of sockets, signal failure to caller
			zproxy_destroy(&self);
		}
	}

	return self;
}

/*===============================================================================*/
static void     *g_sender = NULL;
static void     *g_recver = NULL;
static zproxy_t *g_proxy = NULL;

void zmq_cli_init(char *host, int port)
{	// FIXME:safe and once
	g_sender = zsocket_new(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zsocket_connect(g_sender, addr);
	assert(rc == 0);

	g_recver = zsocket_new(g_ctx, ZMQ_PULL);
#ifdef SELECT_MULTITHREAD
	rc = zsocket_bind(g_recver, "inproc://backend");
#else
	memset(addr, 0, 64);
	sprintf(addr, "ipc://%s-cloud.ipc", g_own);
	rc = zsocket_bind(g_recver, addr);
#endif
	assert(rc == 0);

	g_proxy = zproxy_new(g_ctx, g_recver, g_sender);
	assert(g_proxy);
}

void zmq_cli_spill(struct skt_device *devc, char *data, size_t size)
{
	assert(devc);
	assert(devc->idx < MAX_SPILL_DEPTH);

	if (!devc->skt) {
		devc->skt = zsocket_new(g_ctx, ZMQ_PUSH);
#ifdef SELECT_MULTITHREAD
		int ok = zsocket_connect(devc->skt, "inproc://backend");
#else
		char addr[64] = {};
		sprintf(addr, "ipc://%s-cloud.ipc", g_own);
		int ok = zsocket_connect(devc->skt, addr);
#endif

		if (ok == -1) {
			printf("%s\n", strerror(errno));
		}
	}

	devc->ibuffer[devc->idx].iov_base = data;
	devc->ibuffer[devc->idx].iov_len = size;
	devc->idx++;
}

void zmq_cli_flush(struct skt_device *devc)
{
	int     idx = 0;
	int     off = devc->idx - 1;

	for (idx = 0; idx < off; idx++) {
		struct iovec    *info = &devc->ibuffer[idx];
		int             ok = zsocket_sendmem(devc->skt, info->iov_base, info->iov_len, ZFRAME_MORE);
		assert(ok == 0);
	}

	if (devc->idx != 0) {
		struct iovec    *info = &devc->ibuffer[idx];
		int             ok = zsocket_sendmem(devc->skt, info->iov_base, info->iov_len, 0);
		assert(ok == 0);
	}

	devc->idx = 0;
	memset(devc->ibuffer, 0, sizeof(devc->ibuffer));
}

void zmq_cli_exit(void)
{	// FIXME:safe and once
	zproxy_destroy(&g_proxy);
	zsocket_destroy(g_ctx, g_sender);
	zsocket_destroy(g_ctx, g_recver);
	zctx_destroy(&g_ctx);
}

/*===============================================================================*/
struct skt_kernel       *g_skt_recver = NULL;
struct skt_kernel       *g_skt_router = NULL;

/*
 *   void skt_proxy (void *recver, void *sender)
 *   {
 *        struct iovec ibuffer[32] ;
 *        while (1) {
 *                memset(&ibuffer[0], 0, sizeof(ibuffer));
 *                size_t count = 32;
 *                int rc = zmq_recviov (recver, &ibuffer[0], &count, 0);
 *                if (rc == -1){
 *                        printf("%s\n", strerror (errno));
 *                }else{
 *                        count ++;
 *                        rc = zmq_sendiov (sender, &ibuffer[0], count, ZMQ_SNDMORE);
 *                        int i = 0;
 *                        for(i=0; i<count; i++){
 *                                free (ibuffer[i].iov_base);
 *                        }
 *                }
 *        }
 *
 *   }
 */
void zmq_trs_init(void)
{	// FIXME:safe and once
}

void zmq_trs_add_inport(int port)
{
	void                    *receiver = zsocket_new(g_ctx, ZMQ_PULL);
	struct skt_kernel       *recver = calloc(sizeof(struct skt_kernel), 1);
	struct skt_kernel       *history = g_skt_recver;

	recver->skt = receiver;
	recver->next = history;
	g_skt_recver = recver;

	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", port);
	int rc = zsocket_bind(receiver, addr);
	assert(rc == port);
}

void zmq_trs_add_export(int port)
{
	void                    *publisher = zsocket_new(g_ctx, ZMQ_PUB);
	struct skt_kernel       *router = calloc(sizeof(struct skt_kernel), 1);

	if (g_skt_router == NULL) {
		router->skt = publisher;
		router->next = router;
		g_skt_router = router;
	} else {
		struct skt_kernel *history = g_skt_router->next;
		router->skt = publisher;
		router->next = history;
		g_skt_router->next = router;
	}

	zsocket_set_sndhwm(publisher, 0);
	// zsocket_set_swap (publisher, 25000000);
	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", port);
	int rc = zsocket_bind(publisher, addr);
	assert(rc == port);
}

void zmq_trs_start(void)
{
	zproxy_t *proxy = zproxy_new2(g_ctx, g_skt_recver, g_skt_router);

	assert(proxy);
}

void zmq_trs_exit(void)
{	// FIXME:safe and once
	// zsocket_destroy (g_ctx, publisher);
	zctx_destroy(&g_ctx);
}

/*===============================================================================*/
void    *g_subscriber = NULL;
void    *g_forwarder = NULL;

void zmq_srv_init(char *host, int port)
{	// FIXME:safe and once
	g_subscriber = zsocket_new(g_ctx, ZMQ_SUB);
	zsocket_set_rcvhwm(g_subscriber, 0);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zsocket_connect(g_subscriber, addr);
	assert(rc == 0);

	// char *filter = (argc > 1)? argv [1]: "a";
	char *filter = "";
	zsock_set_subscribe(g_subscriber, filter);

	g_forwarder = zsocket_new(g_ctx, ZMQ_PUSH);
#ifdef SELECT_MULTITHREAD
	rc = zsocket_bind(g_forwarder, "inproc://backend");
#else
	memset(addr, 0, 64);
	sprintf(addr, "ipc://%s-sault.ipc", g_own);
	rc = zsocket_bind(g_forwarder, addr);
#endif
	assert(rc == 0);
}

void zmq_srv_fetch(struct skt_device *devc)
{
	assert(devc);

	if (!devc->skt) {
		devc->skt = zsocket_new(g_ctx, ZMQ_PULL);
#ifdef SELECT_MULTITHREAD
		int ok = zsocket_connect(devc->skt, "inproc://backend");
#else
		char addr[64] = {};
		sprintf(addr, "ipc://%s-sault.ipc", g_own);
		int ok = zsocket_connect(devc->skt, addr);
#endif

		if (ok == -1) {
			printf("%s\n", strerror(errno));
		}
	}

	memset(devc->ibuffer, 0, sizeof(devc->ibuffer));
	devc->idx = 0;
	size_t  size = 0;
	int     more = 0;
	do {
		assert(devc->idx < MAX_SPILL_DEPTH);
		zframe_t *frame = zframe_recv(devc->skt);
		assert(frame);
		assert(zframe_is(frame));
		int rsize = zframe_size(frame);
		assert(rsize != -1);
		byte            *data = zframe_data(frame);
		struct iovec    *info = &devc->ibuffer[devc->idx];
		devc->idx++;
		info->iov_len = rsize;
		info->iov_base = calloc(rsize, 1);
		memcpy(info->iov_base, data, rsize);
		size += rsize;

		more = zframe_more(frame);
		zframe_destroy(&frame);
	} while (more);
}

void zmq_srv_start(void)
{
	zmq_proxy(g_subscriber, g_forwarder, NULL);	// TODO
}

void zmq_srv_exit(void)
{	// FIXME:safe and once
	zsocket_destroy(g_ctx, g_subscriber);
	zctx_destroy(&g_ctx);
}

