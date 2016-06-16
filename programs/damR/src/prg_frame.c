//
//  prg_frame.c
//  supex
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "zmq.h"
#include "prg_frame.h"
#include "load_cfg.h"
#include "data_queue.h"
#include "zmq_wrap.h"
#include "recv_data4zmq.h"
#include "send_data4zmq.h"
#include "ctrl_loop.h"

static void _init_frame(struct frame *frame, struct cfg *cfg, struct queue *queue);

static void _finally_frame(struct frame *frame);

static void _init_proc(struct proc *proc, struct frame *frame);

static void _start_proc(struct proc *proc, void *(*call)(void *));

static void _finally_proc(struct proc *proc);

static void _deal_signal(int signo);

struct frame g_frame = {};

void init(int argc, char **argv)
{
	SetProgName(argv[0]);
	/* ----  */
	load_cfg(argc, argv);
	/* ----  */
	queue_init(&g_queue, &g_cfg);
	/* ----  */
	_init_frame(&g_frame, &g_cfg, &g_queue);
	/* ----  */
	const SLogLevelT *level = NULL;
	level = SLogIntegerToLevel(g_cfg.log.level);
	SLogOpen(g_cfg.log.path, level);
}

void run()
{
	ASSERTOBJ(&g_frame);
	assert(g_frame.stat = FRAME_STAT_INIT);
	struct cfg *cfg = g_frame.cfg;

	if (cfg->recv.proto == PROTO_ZMQ) {
		_start_proc(&g_frame.rcvproc, recv_data4zmq);
	} else {
		RAISE_SYS_ERROR_ERRNO(EPROTO);
	}

	if (cfg->send.proto == PROTO_ZMQ) {
		_start_proc(&g_frame.sndproc, send_data4zmq);
	} else {
		RAISE_SYS_ERROR_ERRNO(EPROTO);
	}

	ctrl_loop(&g_frame);

	SignalIntr(SIGINT, _deal_signal);
	SignalIntr(SIGQUIT, _deal_signal);
}

void stop()
{
	AO_SET((int *)&g_frame.stat, FRAME_STAT_STOP);
	kill(0, SIGINT);
}

void finally()
{
	SignalIntr(SIGINT, SIG_DFL);
	SignalIntr(SIGQUIT, SIG_DFL);
	_finally_frame(&g_frame);
}

/* ---------------------------------------					*/

static void _init_frame(struct frame *frame, struct cfg *cfg, struct queue *queue)
{
	ASSERTOBJ(cfg);
	ASSERTOBJ(queue);
	assert(frame);

	memset(frame, 0, sizeof(*frame));
	frame->cfg = cfg;
	frame->queue = queue;
	frame->stat = FRAME_STAT_INIT;

	TRY
	{
		REFOBJ(frame);

		_init_proc(&frame->rcvproc, frame);
		_init_proc(&frame->sndproc, frame);
	}
	CATCH
	{
		UNREFOBJ(frame);
		RERAISE;
	}
	END;
}

static void _init_proc(struct proc *proc, struct frame *frame)
{
	assert(proc);
	ASSERTOBJ(frame);

	TRY
	{
		memset(proc, 0, sizeof(*proc));
		proc->stat = PROC_STAT_INIT;
		proc->frame = frame;
		proc->tid = -1;
		REFOBJ(proc);
	}
	CATCH
	{
		UNREFOBJ(proc);
		RERAISE;
	}
	END;
}

static void _start_proc(struct proc *proc, void *(*call)(void *))
{
	struct cfg *cfg = NULL;

	ASSERTOBJ(proc);

	assert(proc->stat == PROC_STAT_INIT);

	ASSERTOBJ(proc->frame);
	cfg = proc->frame->cfg;
	ASSERTOBJ(cfg);

	int     flag = 0;
	bool    test = false;
	flag = pthread_create(&proc->ptid, NULL, call, proc);
	RAISE_SYS_ERROR_ERRNO(flag);

	test = futex_cond_wait((int *)&proc->stat, PROC_STAT_RUN, 10);
	AssertError(test, ETIMEDOUT);
}

static void _finally_proc(struct proc *proc)
{
	return_if_fail(UNREFOBJ(proc));

	if (proc->tid > -1) {
		pthread_join(proc->ptid, NULL);
	}

	assert(proc->stat != PROC_STAT_RUN);
	proc->tid = -1;
}

static void _deal_signal(int signo)
{
	AO_CAS((int *)&g_frame.stat, FRAME_STAT_RUN, FRAME_STAT_STOP);

	if (signo == SIGINT) {
		stop_proc(&g_frame.rcvproc);
		stop_proc(&g_frame.sndproc);
	} else if (signo == SIGQUIT) {
		kill_proc(&g_frame.rcvproc);
		kill_proc(&g_frame.sndproc);
	}
}

static void _finally_frame(struct frame *frame)
{
	assert(frame->stat != FRAME_STAT_RUN);
	return_if_fail(UNREFOBJ(frame));

	_finally_proc(&frame->rcvproc);
	_finally_proc(&frame->sndproc);
	queue_finally(frame->queue);
}

