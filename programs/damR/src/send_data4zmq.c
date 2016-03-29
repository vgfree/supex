//
//  send_data4zmq.c
//  supex
//
//  Created by 周凯 on 15/12/7.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "zmq.h"
#include "send_data4zmq.h"
#include "zmq_wrap.h"
#include "connection_pool.h"

#define SNDDATA_STACK_SIZE (4096)

static void send_idle(struct evcoro_scheduler *scheduler, void *usr);

/*接收发送请求数据包*/
static void recv_data(struct evcoro_scheduler *scheduler, void *usr);

static void send_data(struct evcoro_scheduler *scheduler, void *usr);

void *send_data4zmq(void *arg)
{
	struct proc     *proc = arg;
	struct frame    *frame = NULL;
	struct cfg      *cfg = NULL;

	ASSERTOBJ(proc);
	frame = proc->frame;
	ASSERTOBJ(frame);
	cfg = frame->cfg;
	ASSERTOBJ(cfg);

	assert(proc->stat = PROC_STAT_INIT);
	assert(cfg->send.proto == PROTO_ZMQ);
	struct iohandle *volatile io = NULL;

	pthread_cleanup_push((void (*)(void *))stop_proc, proc);

	TRY
	{
		bool flag = false;
		proc->tid = GetThreadID();
		proc->coroloop = evcoro_create(-1);
		AssertError(proc->coroloop, ENOMEM);

		futex_set_signal((int *)&proc->stat, PROC_STAT_RUN, -1);
		flag = futex_cond_wait((int *)&frame->stat, FRAME_STAT_RUN, 10);
		if (unlikely(!flag)) ReturnValue(NULL);

		New(io);
		AssertError(io, ENOMEM);

		/*绑定连接*/
		io->linger = 2 * 1000;
		io->type = SOCK_ZMQ_BIND;
		io->zmq.ctx = zmq_ctx_new();
		AssertError(io->zmq.ctx, ENOMEM);
		io->zmq.skt = zmq_socket(io->zmq.ctx, ZMQ_PULL);
		AssertRaise(io->zmq.skt, EXCEPT_SYS);

		set_zmqopt(io->zmq.skt, ZMQ_RCVHWM, 100);

		int rc = 0;
		snprintf(io->name, sizeof(io->name), "tcp://%s:%d", cfg->send.ip, cfg->send.port);
		rc = zmq_bind(io->zmq.skt, io->name);
		RAISE_SYS_ERROR(rc);

		io->zmq.fd = get_zmqopt(io->zmq.skt, ZMQ_FD);

		x_printf(D, "send data thread `%ld` start ...", proc->tid);

		evcoro_set_usrdata(proc->coroloop, proc);
		flag = evcoro_push(proc->coroloop, recv_data, io, SNDDATA_STACK_SIZE);
		AssertError(flag, ENOMEM);

		evcoro_loop(proc->coroloop, send_idle, proc);
	}
	CATCH
	{}
	FINALLY
	{
		evcoro_destroy(proc->coroloop, NULL);
		iohandle_destroy(io);
		
		x_printf(E, "send data thread `%ld` end.", proc->tid);
	}
	END;

	pthread_cleanup_pop(true);

	return NULL;
}

static void send_idle(struct evcoro_scheduler *scheduler, void *usr)
{
	struct proc     *proc = usr;
	struct frame    *frame = proc->frame;
	struct queue    *queue = frame->queue;

	x_printf(D, "check status of system.");
	
	ASSERTOBJ(queue);

	if (unlikely((frame->stat != FRAME_STAT_RUN) ||
		evcoro_actives(scheduler) < 1)) {
		evcoro_stop(proc->coroloop);
	} else {
		futex_wait(queue->nodes, 0, 1000);
	}
}

/*接收发送请求数据包*/
static void recv_data(struct evcoro_scheduler *scheduler, void *usr)
{
	struct iohandle				*io = usr;
	struct proc					*proc = NULL;
	struct frame				*frame = NULL;
	struct zmqframe *volatile	zframe = NULL;
	struct  reqconn				*reqconn = NULL;
	
	assert(io);
	proc = evcoro_get_usrdata(scheduler);
	frame = proc->frame;
	zframe = zmqframe_new(sizeof(struct reqconn));
	AssertError(zframe, ENOMEM);
	reqconn = (struct reqconn *)zframe->data;
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)zmqframe_free, zframe);

	x_printf(W, "receive request data start.");
	do {
		int zframes = 0;
		/*新的连接*/
		UNREFOBJ(zframe);
		zframes = read_zmqdata(io->zmq.skt, zframe);
		RAISE_SYS_ERROR(zframes);

		if (likely(zframe->frames == 1 && sizeof(*reqconn) == zframe->frame[0])) {
			struct  iohandle	*conn = NULL;
			union evcoro_event	tevent = {};
			
			New(conn);
			if (unlikely(!conn)) {
				x_printf(W, "processor has used too much memory");
				continue;
			}
			
			int byte = 0;
			byte = snprintf(conn->name, sizeof(conn->name), "tcp://%s:%d", reqconn->ipaddr, ntohs(reqconn->port));
			AssertError(byte < sizeof(conn->name), ENOMEM);
			
			x_printf(D, "new request of connection `%s`", conn->name);
			
			/*延迟关闭*/
			conn->linger = 2 * 1000;
			conn->type = SOCK_ZMQ_CONN;
			conn->zmq.ctx = io->zmq.ctx;
			conn->usr = proc;
			
			bool flag = false;
			flag = add_connection(conn);
			
			if (unlikely(flag)) {
				/*新连接，则新增一个任务处理*/
				while (1) {
					flag = evcoro_push(scheduler, send_data, conn, 0);
					if (likely(flag)) break;
					evcoro_timer_init(&tevent, .5);
					/*为防止切出后，协程随循环直接退出，则压入清理*/
					evcoro_cleanup_push(scheduler, (evcoro_destroycb)iohandle_destroy, conn);
					evcoro_idleswitch(scheduler, &tevent, EVCORO_TIMER);
					/*切回后，弹出，但是不执行清理*/
					evcoro_cleanup_pop(scheduler, false);
				}
			} else {
				x_printf(D, "this connection `%s` is exist", conn->name);
				/*已存在或超过最大连接数*/
				iohandle_destroy(conn);
			}

			/*为了提高效率，每.01秒接受一个新的连接请求*/
			evcoro_timer_init(&tevent, .05);
			evcoro_idleswitch(scheduler, &tevent, EVCORO_TIMER);
		} else if (likely(zframes == 0)) {
			union evcoro_event revent = {};
			evcoro_io_init(&revent, io->zmq.fd, 1);
			// slowly switch to another coroutine
			evcoro_idleswitch(scheduler, &revent, EVCORO_READ);
		} else {
			errno = EPROTO;
			x_printf(E, "receive request data failed : %s", x_strerror(EPROTO));
			break;
		}
	} while (1);

	evcoro_cleanup_pop(scheduler, true);
	
	x_printf(W, "receive request data over.");
}

static void send_data(struct evcoro_scheduler *scheduler, void *usr)
{
	struct iohandle         *io = usr;
	struct frame            *frame = NULL;
	struct queue            *mq = NULL;
	struct zmqframe         *zframe = NULL;
	struct proc             *proc = io->usr;

	assert(io);
	ASSERTOBJ(proc);
	frame = proc->frame;
	ASSERTOBJ(frame);
	mq = frame->queue;
	ASSERTOBJ(mq);
	
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)iohandle_destroy, io);
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)delete_connection, io);

	zframe = zmqframe_new(mq->cellsize);
	AssertError(zframe, ENOMEM);
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)zmqframe_free, zframe);
	
	x_printf(D, "start connect to %s", io->name);
	
	/*连接对端*/
	io->zmq.skt = zmq_socket(io->zmq.ctx, ZMQ_PUSH);
	AssertRaise(io->zmq.skt, EXCEPT_SYS);

	set_zmqopt(io->zmq.skt, ZMQ_IMMEDIATE, 1);
	set_zmqopt(io->zmq.skt, ZMQ_SNDHWM, frame->cfg->send.cache);

	int rc = 0;
	rc = zmq_connect(io->zmq.skt, io->name);
	if (unlikely(rc < 0)) {
		/*连接发送错误，断开连接*/
		x_perror("zmq_connect() by `%s` error : %s", io->name, x_strerror(errno));
		return;
	}
	
	io->zmq.fd = get_zmqopt(io->zmq.skt, ZMQ_FD);
	/*测试是否已经连接*/
	union evcoro_event	wevent = {};
	bool				flag = false;
	evcoro_io_init(&wevent, io->zmq.fd, .1);
	
	flag = evcoro_idleswitch(scheduler, &wevent, EVCORO_WRITE);
	if (unlikely(!flag)) {
		x_printf(W, "connect to %s timed out", io->name);
		return;
	}

	/*开始发送*/
	do {
		int size = 0;
		int zframes = 0;
		/*弹出一个待发送数据*/
		flag = mq->pull(mq->queue, (char *)zframe, zframe->size, &size);

		if (likely(flag)) {
			AssertError(size <= zframe->size, EMSGSIZE);
			futex_wake(mq->nodes, -1);
		} else {
			evcoro_fastswitch(scheduler);
			continue;
		}

		/*发送*/
snd_again:
		zframes = write_zmqdata(io->zmq.skt, zframe);

		if (likely(zframes > 0)) {
			evcoro_fastswitch(scheduler);
		} else if (unlikely(zframes == 0)) {
			union evcoro_event tevent = {};
			evcoro_timer_init(&tevent, .001);
			// slowly switch to another coroutine
			evcoro_idleswitch(scheduler, &tevent, EVCORO_TIMER);
			flag = check_zmqwrite(io->zmq.skt, 0);
			if (likely(flag)) {
				goto snd_again;
			} else {
				x_printf(W, "write data to %s timed out", io->name);
				/*返还数据到队列，并结束本次发送*/
				mq->priopush(mq->queue, (char *)zframe, zframe->size, NULL);
				break;
			}
		} else {
			x_perror("write_zmqdata() by `%s` error : %s", io->name, x_strerror(errno));
			break;
		}
	} while (1);

	x_printf(D, "disconnect `%s`", io->name);

	evcoro_cleanup_pop(scheduler, true);
	evcoro_cleanup_pop(scheduler, true);
	evcoro_cleanup_pop(scheduler, true);
}

