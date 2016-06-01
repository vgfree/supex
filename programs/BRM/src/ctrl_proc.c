//
//  ctrl_proc.c
//  supex
//
//  Created by 周凯 on 15/10/29.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "ev.h"
#include "data_model.h"
#include "parse_cfg.h"
#include "prg_frame.h"
#include "ctrl_proc.h"
#include "tcp_api.h"

#define USE_EV_TIMER

static void _setsktopt(int fd, void *usr);

static int _connect_noblock(const char *host, int port, int to);

static void _ctrl_reload_cfg(struct ev_loop *loop, ev_stat *stat, int event);

static void _ctrl_signal_hdl(struct ev_loop *loop, ev_signal *signal, int event);

static void _stop_procentry(struct procentry *proc);

static void _kill_procentry(struct procentry *proc);

#ifndef USE_EV_TIMER
static ev_tstamp _timer_scheduler(ev_periodic *interval, ev_tstamp now);
static void _ctrl_interval(struct ev_loop *loop, ev_periodic *interval, int event);
#else
static void _ctrl_timeout(struct ev_loop *loop, ev_timer *timer, int event);
#endif

void main_loop(struct framentry *frame)
{
	assert(frame);

	TRY
	{
		struct ev_loop *loop = NULL;

		loop = ev_default_loop(EVFLAG_AUTO | EVBACKEND_EPOLL);
		AssertError(loop, ENOMEM);

		/*start work*/
		ev_signal       sigintr = {};
		ev_signal       sigpipe = {};
		ev_signal       sigquit = {};
		ev_stat         filewatcher = {};

		sigintr.data = frame;
		sigpipe.data = frame;
		sigpipe.data = frame;
		filewatcher.data = frame;

		ev_stat_init(&filewatcher, _ctrl_reload_cfg, frame->cfg->cfgfile, 1.0);
		ev_signal_init(&sigintr, _ctrl_signal_hdl, SIGINT);
		ev_signal_init(&sigpipe, _ctrl_signal_hdl, SIGPIPE);
		ev_signal_init(&sigquit, _ctrl_signal_hdl, SIGQUIT);

		ev_stat_start(loop, &filewatcher);
		ev_signal_start(loop, &sigintr);
		ev_signal_start(loop, &sigpipe);
		ev_signal_start(loop, &sigquit);

		// 时间间隔触发
#ifndef USE_EV_TIMER
		ev_periodic interval = {};
		ev_periodic_init(&interval, _ctrl_interval, 0., 0., _timer_scheduler);
		ev_periodic_start(loop, &interval);
#else
		ev_timer timer = { };
		ev_timer_init(&timer, _ctrl_timeout, 1.0, .0);
		ev_timer_start(loop, &timer);
#endif
		ev_set_userdata(loop, frame);

		// wake up all of waiter
		futex_set_signal((int *)&frame->stat, FRAME_STAT_RUN, -1);

		ev_loop(loop, 0);

		ev_loop_destroy(loop);
	}
	CATCH
	{
		RERAISE;
	}
	FINALLY
	{
		AO_SET(&frame->stat, FRAME_STAT_STOP);
	}
	END;
}

static void _ctrl_reload_cfg(struct ev_loop *loop, ev_stat *stat, int event)
{
	struct framentry *frame = stat->data;

	assert(frame);

	/*
	 * 是否是修改触发
	 */
	if (stat->attr.st_mtime == stat->prev.st_mtime) {
		return;
	}

	x_printf(W, "configure file %s has been reload.", stat->path);

	/*
	 * 创建挂起系统句柄
	 */
	struct ThreadSuspend volatile   cond = NULLOBJ_THREADSUSPEND;
	int                             counter = 0;
	struct allcfg volatile          newcfg = {};
	cJSON *volatile                 json = NULL;
	bool                            flag = false;
	int volatile                    ptasks = frame->cfg->paralleltasks;

	TRY
	{
		/*
		 * 暂停任务，将并发任务数变为零，路由线程因此不会从队列中弹出任务
		 */
		AO_SET(&frame->cfg->paralleltasks, 0);

		/*发送异步事件，唤醒挂起任务*/
		for (counter = 0; counter < frame->routeprocs; counter++) {
			struct procentry *ptr = &frame->routeproc[counter];
			assert(ptr->type == PROC_TYPE_EVENT);
			ptr->ctrl.data = (void *)&cond;
			ev_async_send(ptr->evloop, &ptr->ctrl);
		}

		/*等待挂起*/
		ThreadSuspendWait((struct ThreadSuspend *)&cond, counter);

		/*重读配置文件*/
		json = load_cfg2json(frame->cfg->cfgfile);
		AssertRaise(json, EXCEPT_LOADCFG_FAIL);

		flag = load_json2allcfg(json, (struct allcfg *)&newcfg);
		AssertRaise(flag, EXCEPT_LOADCFG_FAIL);

		/*打开日志文件*/
		const SLogLevelT *level = NULL;
		level = SLogIntegerToLevel(newcfg.loglevel);
		flag = SLogOpen(newcfg.logpath, level);
		AssertRaise(flag, EXCEPT_LOADCFG_FAIL);

		/*初始化新的连接池，不能更改并发任务的数量*/
		newcfg.paralleltasks = ptasks;
		newcfg.threads = frame->cfg->threads;
		flag = cntpool_init((struct allcfg *)&newcfg);
		AssertRaise(flag, EXCEPT_LOADCFG_FAIL);

		/*设置新的配置*/
		load_reload((struct allcfg *)&newcfg, frame->cfg);
	}
	CATCH
	{
		allcfg_destroy((struct allcfg *)&newcfg);
	}
	FINALLY
	{
		/*结束挂起*/
		ThreadSuspendEnd((struct ThreadSuspend *)&cond);

		AO_SET(&frame->cfg->paralleltasks, ptasks);

		for (counter = 0; counter < frame->routeprocs; counter++) {
			struct procentry *ptr = &frame->routeproc[counter];
			assert(ptr->type == PROC_TYPE_EVENT);
			ptr->ctrl.data = NULL;
		}

		Free(json);
	}
	END;
}


static void _setsktopt(int fd, void *usr)
{
	int *timeout = usr;
	
	SO_SetSndTimeout(fd, *timeout);
}

static int _connect_noblock(const char *host, int port, int to)
{
	int     fd = -1;
	char    ports[7] = {};
	
	snprintf(ports, sizeof(ports), "%d", port);
	fd = TcpConnect(host, ports, _setsktopt, &to);
	return fd;
}

#ifndef USE_EV_TIMER
static ev_tstamp _timer_scheduler(ev_periodic *interval, ev_tstamp now)
{
	return now + 1;
}

static void _ctrl_interval(struct ev_loop *loop, ev_periodic *interval, int event)
#else
static void _ctrl_timeout(struct ev_loop *loop, ev_timer *timer, int event)
#endif
{
	struct framentry *frame = ev_userdata(loop);

	assert(frame);

	if (unlikely(frame->stat != FRAME_STAT_RUN)) {
#ifndef USE_EV_TIMER
		ev_periodic_stop(loop, interval);
#else
		ev_timer_stop(loop, timer);
#endif
		kill(0, SIGINT);
		ev_feed_signal_event(loop, SIGINT);
		ev_break(loop, EVBREAK_ALL);
	}
	
	TRY
	{
		/*检查失效的主机*/
		int i = 0;
		int max = 0;
		struct allcfg *cfg = frame->cfg;
		struct hostentry *hent = cfg->host.calhost.host;
		max = cfg->host.calhost.hosts;
		for (i = 0; cfg->calculate && hent && i < max; i++) {
			int fd = 0;
			int errconn = hent[i].errconn;
			if (likely(errconn == 0)) continue;
//			fd = _connect_noblock(hent[i].ip, hent[i].port, cfg->idlesleep);
			fd = x_connect(hent[i].ip, hent[i].port, cfg->idlesleep);
			if (unlikely(fd < 0)) continue;
			AO_CAS(&hent[i].errconn, errconn, 0);
			close(fd);
		}
		
		struct hostgroup *hgrp = cfg->host.routehost.hostgrp;
		max = cfg->host.routehost.hostgrps;
		for (i = 0; hgrp && i < max; i++) {
			int j = 0;
			int max2 = hgrp[i].hosts;
			struct hostentry *hent = hgrp[i].host;
			for (j = 0; hent && j < max2; j++) {
				int fd = 0;
				int errconn = hent[j].errconn;
				if (likely(errconn == 0)) continue;
//				fd = _connect_noblock(hent[j].ip, hent[j].port, cfg->idlesleep);
				fd = x_connect(hent[j].ip, hent[j].port, cfg->idlesleep);
				if (unlikely(fd < 0)) continue;
				AO_CAS(&hent[j].errconn, errconn, 0);
				close(fd);
			}
		}
	}
	CATCH
	{}
	END;
	
#ifdef USE_EV_TIMER
	ev_timer_stop(loop, timer);
	ev_timer_init(timer, _ctrl_timeout, 1.0, .0);
	ev_timer_start(loop, timer);
#endif
}

static void _ctrl_signal_hdl(struct ev_loop *loop, ev_signal *signal, int event)
{
	struct framentry *frame = ev_userdata(loop);

	assert(frame);

	if (signal->signum == SIGPIPE) {
		x_printf(W, "receive SIGPIPE !!!");
		return;
	} else if (signal->signum == SIGINT) {
		return_if_fail(AO_CASB((int *)&frame->stat,
			FRAME_STAT_RUN,
			FRAME_STAT_STOP));
		int i = 0;

		for (i = 0; i < frame->routeprocs; i++) {
			_stop_procentry(&frame->routeproc[i]);
		}

		_stop_procentry(&frame->recvproc);

		ev_break(loop, EVBREAK_ALL);
	} else if (signal->signum == SIGQUIT) {
		int i = 0;

		for (i = 0; i < frame->routeprocs; i++) {
			_kill_procentry(&frame->routeproc[i]);
		}

		_kill_procentry(&frame->recvproc);

		ev_break(loop, EVBREAK_ALL);
	}
}

static void _stop_procentry(struct procentry *proc)
{
	return_if_fail(proc && proc->evloop && proc->corloop);

	if (proc->type == PROC_TYPE_CORO) {
		evcoro_stop(proc->corloop);
	} else {
		ev_break(proc->evloop, EVBREAK_ALL);
	}
}

static void _kill_procentry(struct procentry *proc)
{
	return_if_fail(proc && proc->evloop && proc->corloop);

	if (proc->stat != PROC_STAT_STOP) {
		pthread_cancel(proc->ptid);
	}
}

