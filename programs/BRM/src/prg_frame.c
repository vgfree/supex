//
//  prg_frame.c
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "parse_cfg.h"
#include "pool_api/conn_xpool_api.h"
#include "recv_data.h"
#include "route_data.h"
#include "ctrl_proc.h"
#include "prg_frame.h"

static void _init_framentry(struct allcfg *cfg);

static void _finally_framentry();

static bool _init_procentry(struct allcfg *cfg, struct procentry *proc, int type);

static void _stop_procentry(struct procentry *proc);

static void _kill_procentry(struct procentry *proc);

static void _finally_procentry(struct procentry *proc);

static void _deal_signal(int signo);

void initialize(int argc, char **argv)
{
	const SLogLevelT *level = NULL;

	TRY
	{
		bool flag = false;

		SetProgName(argv[0]);

		load_cfg(argc, argv);

		// initialize the pool of connection.
		flag = cntpool_init(&g_allcfg);
		AssertError(flag, ECONNREFUSED);

		// initialize frame
		_init_framentry(&g_allcfg);

		// open log
		level = SLogIntegerToLevel(g_allcfg.loglevel);
		SLogOpen(g_allcfg.logpath, level);
	}
	CATCH
	{
		RERAISE;
	}
	END;
}

void run()
{
	int     flag = 0;
	int     i = 0;

	assert(g_framentry.stat == FRAME_STAT_INIT);

	// start receive process
	flag = pthread_create(&g_framentry.recvproc.ptid, NULL,
			recv_data, &g_framentry.recvproc);
	RAISE_SYS_ERROR_ERRNO(flag);

	futex_cond_wait((int *)&g_framentry.recvproc.stat, PROC_STAT_RUN, -1);

	for (i = 0; i < g_framentry.routeprocs; i++) {
		flag = pthread_create(&g_framentry.routeproc[i].ptid, NULL,
				route_data, &g_framentry.routeproc[i]);
		RAISE_SYS_ERROR_ERRNO(flag);

		futex_cond_wait((int *)&g_framentry.routeproc[i].stat, PROC_STAT_RUN, -1);
	}

	// main loop
	main_loop(&g_framentry);

	// handle signal for SIGINT
	SignalIntr(SIGINT, _deal_signal);
	SignalIntr(SIGQUIT, _deal_signal);
}

void stop()
{
	AO_SET((int *)&g_framentry.stat, FRAME_STAT_STOP);
	kill(0, SIGINT);
}

void finally()
{
	int i = 0;

	for (i = 0; i < g_framentry.routeprocs; i++) {
		if (g_framentry.routeproc[i].tid > 0) {
			pthread_join(g_framentry.routeproc[i].ptid, NULL);
		}
	}

	if (g_framentry.recvproc.tid > 0) {
		pthread_join(g_framentry.recvproc.ptid, NULL);
	}

	_finally_framentry();
}

bool cntpool_init(struct allcfg *cfg)
{
	bool    flag = false;
	int     poolsize = 0;

	assert(cfg);

	poolsize = cfg->paralleltasks / cfg->threads;
	poolsize = MAX(poolsize, 32);
//	poolsize = INRANGE(poolsize, 32, 1024);
	// calculate host
	if (cfg->calculate) {
		flag = hostgroup_connect(&cfg->host.calhost, poolsize);
		return_val_if_fail(flag, false);
	}
	// route table host
	flag = hostcluster_connect(&cfg->host.routehost, poolsize);
	// 路由主机初始化失败是否返回错误
//	return_val_if_fail(flag, false);
	
	return true;
}

static void _init_framentry(struct allcfg *cfg)
{
	assert(cfg);

	int     i = 0;
	bool    flag = 0;

	g_framentry.cfg = cfg;

	g_framentry.queue = MEM_QueueCreate(NULL, cfg->queuesize, sizeof(uintptr_t));
	AssertError(g_framentry.queue, ENOMEM);

	g_framentry.routeprocs = cfg->threads;
	NewArray0(g_framentry.routeprocs, g_framentry.routeproc);
	AssertError(g_framentry.routeproc, ENOMEM);

	for (i = 0; i < g_framentry.routeprocs; i++) {
		flag = _init_procentry(cfg, &g_framentry.routeproc[i], PROC_TYPE_EVENT);
		assert(flag);
	}

	flag = _init_procentry(cfg, &g_framentry.recvproc, PROC_TYPE_CORO);
	assert(flag);

	// 放在最后
	g_framentry.stat = FRAME_STAT_INIT;
}

static void _finally_framentry()
{
	assert(g_framentry.stat != FRAME_STAT_RUN);

	SignalIntr(SIGINT, SIG_DFL);
	SignalIntr(SIGQUIT, SIG_DFL);
	
	if (g_framentry.routeproc) {
		int i = 0;

		for (i = 0; i < g_framentry.routeprocs; i++) {
			_finally_procentry(&g_framentry.routeproc[i]);
		}
	}

	_finally_procentry(&g_framentry.recvproc);

	Free(g_framentry.routeproc);
	g_framentry.stat = FRAME_STAT_NONE;

	MEM_QueueDestroy(&g_framentry.queue, false, NULL);
}

static bool _init_procentry(struct allcfg *cfg, struct procentry *proc, int type)
{
	assert(cfg && proc);

	proc->frame = &g_framentry;
	proc->cfg = cfg;
	proc->tid = -1;
	proc->type = type;

	if (type == PROC_TYPE_CORO) {
		//不能在非使用的线程创建协同
	} else {
		proc->type = PROC_TYPE_EVENT;
		proc->evloop = ev_loop_new(EVBACKEND_EPOLL);
		return_val_if_fail(proc->evloop, false);
	}

	return true;
}

static void _finally_procentry(struct procentry *proc)
{
	return_if_fail(proc && proc->evloop && proc->corloop);

	if (proc->type == PROC_TYPE_CORO) {
		//不能在非使用的线程销毁协同
	} else {
		ev_break(proc->evloop, EVBREAK_ALL);
		ev_loop_destroy(proc->evloop);
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

static void _deal_signal(int signo)
{
	AO_CAS((int *)&g_framentry.stat, FRAME_STAT_RUN, FRAME_STAT_STOP);

	int i = 0;

	for (i = 0; i < g_framentry.routeprocs; i++) {
		if (signo == SIGINT) {
			_stop_procentry(&g_framentry.routeproc[i]);
		} else if (signo == SIGQUIT) {
			_kill_procentry(&g_framentry.routeproc[i]);
		}
	}

	if (signo == SIGINT) {
		_stop_procentry(&g_framentry.recvproc);
	} else if (signo == SIGQUIT) {
		_kill_procentry(&g_framentry.recvproc);
	}
}

