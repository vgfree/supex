//
//  ctrl_loop.c
//  supex
//
//  Created by 周凯 on 15/12/7.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "ctrl_loop.h"

static void _ctrl_signal(struct ev_loop *loop, ev_signal *signal, int event);

static void _ctrl_idle(struct ev_loop *loop, ev_idle *idle, int event);

static void _ctrl_interval(struct ev_loop *loop, ev_periodic *interval, int event);

static ev_tstamp _timer_scheduler(ev_periodic *interval, ev_tstamp now);

void ctrl_loop(struct frame *frame)
{
	ASSERTOBJ(frame);

	struct ev_loop *volatile loop = NULL;

	TRY
	{
		loop = ev_default_loop(EVFLAG_AUTO | EVBACKEND_EPOLL);
		AssertError(loop, ENOMEM);

		ev_signal       sigintr = {};
		ev_signal       sigpipe = {};
		ev_signal       sigquit = {};

		sigintr.data = frame;
		sigpipe.data = frame;
		sigpipe.data = frame;

		ev_signal_init(&sigintr, _ctrl_signal, SIGINT);
		ev_signal_init(&sigpipe, _ctrl_signal, SIGPIPE);
		ev_signal_init(&sigquit, _ctrl_signal, SIGQUIT);
		ev_signal_start(loop, &sigintr);
		ev_signal_start(loop, &sigpipe);
		ev_signal_start(loop, &sigquit);

		ev_idle idle = {};
		ev_idle_init(&idle, _ctrl_idle);
		//		ev_idle_start(loop, &idle);

		// 时间间隔触发
		ev_periodic interval = {};
		ev_periodic_init(&interval, _ctrl_interval, 0., 0., _timer_scheduler);
		ev_periodic_start(loop, &interval);
		//
		ev_set_userdata(loop, frame);
		futex_set_signal((int *)&frame->stat, FRAME_STAT_RUN, -1);
		ev_loop(loop, 0);
	}
	CATCH
	{
		RERAISE;
	}
	FINALLY
	{
		if (likely(loop)) {
			ev_loop_destroy(loop);
		}

		ATOMIC_SET(&frame->stat, FRAME_STAT_STOP);
	}
	END;
}

static void _ctrl_signal(struct ev_loop *loop, ev_signal *signal, int event)
{
	struct frame *frame = ev_userdata(loop);

	assert(frame);

	if (signal->signum == SIGPIPE) {
		x_printf(W, "receive SIGPIPE !!!");
		return;
	} else if (signal->signum == SIGINT) {
		ATOMIC_CASB((int *)&frame->stat, FRAME_STAT_RUN, FRAME_STAT_STOP);
		stop_proc(&frame->rcvproc);
		stop_proc(&frame->sndproc);
		ev_break(loop, EVBREAK_ALL);
	} else if (signal->signum == SIGQUIT) {
		kill_proc(&frame->rcvproc);
		kill_proc(&frame->sndproc);
		ev_break(loop, EVBREAK_ALL);
	}
}

static void _ctrl_idle(struct ev_loop *loop, ev_idle *idle, int event)
{
	//
}

static void _ctrl_interval(struct ev_loop *loop, ev_periodic *interval, int event)
{
	struct frame *frame = ev_userdata(loop);

	assert(frame);

	if (unlikely(frame->stat != FRAME_STAT_RUN)) {
		ev_periodic_stop(loop, interval);
		kill(0, SIGINT);
		ev_feed_signal_event(loop, SIGINT);
	}
}

static ev_tstamp _timer_scheduler(ev_periodic *interval, ev_tstamp now)
{
	return now + 1;
}

