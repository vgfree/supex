//
//  recv_data4zmq.c
//  supex
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "zmq.h"
#include "recv_data4zmq.h"
#include "zmq_wrap.h"

#define RCVDATA_STACK_SIZE (4096)

static void recv_idle(struct evcoro_scheduler *scheduler, void *usr);

static void recv_data(struct evcoro_scheduler *scheduler, void *usr);

void *recv_data4zmq(void *arg)
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
	assert(cfg->recv.proto == PROTO_ZMQ);

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

		if (unlikely(!flag)) {
			ReturnValue(NULL);
		}

		New(io);
		AssertError(io, ENOMEM);

		io->linger = 0;
		io->type = SOCK_ZMQ_BIND;
		io->zmq.ctx = zmq_ctx_new();
		AssertError(io->zmq.ctx, ENOMEM);
		io->zmq.skt = zmq_socket(io->zmq.ctx, ZMQ_PULL);
		AssertRaise(io->zmq.skt, EXCEPT_SYS);

		set_zmqopt(io->zmq.skt, ZMQ_RCVHWM, frame->cfg->recv.cache);

		int rc = 0;
		snprintf(io->name, sizeof(io->name), "tcp://%s:%d", cfg->recv.ip, cfg->recv.port);
		rc = zmq_bind(io->zmq.skt, io->name);
		RAISE_SYS_ERROR(rc);
		io->zmq.fd = get_zmqopt(io->zmq.skt, ZMQ_FD);

		x_printf(D, "receive data thread `%ld` begin ...", proc->tid);

		evcoro_set_usrdata(proc->coroloop, proc);
		flag = evcoro_push(proc->coroloop, recv_data, io, RCVDATA_STACK_SIZE);
		AssertError(flag, ENOMEM);

		evcoro_loop(proc->coroloop, recv_idle, proc);
	}
	CATCH
	{}
	FINALLY
	{
		evcoro_destroy(proc->coroloop, NULL);
		iohandle_destroy(io);
		x_printf(D, "receive data thread `%ld` end.", proc->tid);
	}
	END;

	pthread_cleanup_pop(true);

	return NULL;
}

static void recv_idle(struct evcoro_scheduler *scheduler, void *usr)
{
	struct proc     *proc = usr;
	struct frame    *frame = proc->frame;
	struct queue    *queue = frame->queue;

	x_printf(D, "check status of system.");

	ASSERTOBJ(queue);

	if (unlikely((frame->stat != FRAME_STAT_RUN) ||
		(evcoro_actives(scheduler) < 1))) {
		evcoro_stop(scheduler);
	} else {
		int flag = 0;
		flag = futex_wait(queue->nodes, queue->capacity, 1000);

		if (likely(flag == 0)) {
			x_printf(W, "the space of queue is too small.");
		}
	}
}

static void recv_data(struct evcoro_scheduler *scheduler, void *usr)
{
	struct proc                     *proc = evcoro_get_usrdata(scheduler);
	struct iohandle                 *io = usr;
	struct frame                    *frame = NULL;
	struct queue                    *mq = NULL;
	struct cfg                      *cfg = NULL;
	struct zmqframe *volatile       zframe = NULL;
	int                             zframes = 0;

	frame = proc->frame;
	mq = frame->queue;
	cfg = frame->cfg;

	ASSERTOBJ(mq);
	ASSERTOBJ(cfg);

	zframe = zmqframe_new(cfg->queue.cellsize);
	AssertError(zframe, ENOMEM);

	evcoro_cleanup_push(scheduler, (evcoro_destroycb)zmqframe_free, zframe);
	x_printf(D, "start receive data ...");

	do {
		zframes = read_zmqdata(io->zmq.skt, zframe);

		if (likely(zframes > 0)) {
			int     size = 0;
			bool    flag = false;

			do {
				// push into (file) queue
				flag = mq->push(mq->queue, (char *)zframe, zframe->offset, &size);

				if (likely(flag)) {
					break;
				}

				evcoro_fastswitch(scheduler);
			} while (1);

			AssertError(size == zframe->offset, ENOMEM);
			futex_wake(mq->nodes, -1);
			UNREFOBJ(zframe);
			x_printf(D, "receive `%d` frames data.", zframes);
		} else if (likely(zframes == 0)) {
			union evcoro_event event = {};
			evcoro_io_init(&event, io->zmq.fd, 1);
			// slowly switch to another coroutine
			evcoro_idleswitch(scheduler, &event, EVCORO_READ);
		} else {
			x_perror("read_zmqdata() from %s error : %s", io->name, x_strerror(errno));
		}
	} while (1);

	x_printf(E, "stop receive data ...");

	evcoro_cleanup_pop(scheduler, true);
	evcoro_stop(scheduler);
}

