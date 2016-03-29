//
//  data_model.h
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef data_model_h
#define data_model_h

#include <stdio.h>
#include "evcoro_scheduler.h"
#include "ev.h"
// #include "zmq.h"
#include "cJSON.h"
//#include "redis_parser.h"
#include "http.h"
#include "cnt_pool.h"
#include "async_obj.h"
#include "except_info.h"

#include "cache/cache.h"
#include "libmini.h"



enum proto
{
	PROTO_HTTP = 0x01,
	PROTO_REDIS,
	PROTO_JSON
};

/* -----------                          */
struct hostentry
{
	char            *name;
	char            *url;		// used to request data by http protocol
	char            *hostaddr;	// 127.0.0.1:1000
	char            *ip;		// 127.0.0.1
	int             port;		// 1000
	int				errconn;	// counter about invalid connection
	enum proto_type proto;		// protocol
};

struct hostgroup
{
	char                    *name;	// name of hosts-group/
	int                     hosts;	// number of hosts-group of hosts
	struct hostentry        *host;	// hosts-group
};

struct hostcluster
{
	char                    *name;
	int                     hostgrps;
	struct hostgroup        *hostgrp;
};

struct hostclusters
{
	char                    *name;
	int                     hostclusters;
	struct hostcluster      *hostcluster;
};

struct hostcfg
{
	struct hostcluster      srchost;// source-data'host
	struct hostgroup        calhost;
	struct hostcluster      routehost;
};

/* -----------                          */
struct taghash
{
	char                    *name;
	int                     tag;
	HashCB                  hash;
	struct hostgroup        *hostgrp;
};

struct datasrc
{
	char            *name;
	int             taghashs;
	struct taghash  *taghash;
};

struct routerule
{
	char            *name;
	int             datasrcs;
	struct datasrc  *datasrc;
};
/* -----------                          */

/* -----------                          */
struct allcfg
{
	struct routerule        routerule;	/**<路由规则*/
	struct hostcfg          host;		/**<主机配置*/
	short                   loglevel;	/**<日志级别*/
	char                    *logpath;	/**<日志文件路径*/
	int                     threads;	/**<路由线程数*/
	int                     paralleltasks;	/**<并发任务数*/
	char                    *cfgfile;	/**<配置文件名称*/
	int                     queuesize;	/**< 接收队列大小*/
	int                     pkgsize;	/**< 数据包大小*/
	bool					calculate; /**< 是否进行业务计算*/
	int						idlesleep; /**< 空闲休眠时间*/
	bool					ischeckfd; /**< 在读写前是否检查描述符有效*/
};

/* -----------                          */

struct taskdata;
struct framentry;

struct netdata
{
	enum proto_type         proto;
	cJSON                   *json;
	struct cnt_pool         *cntpool;
	intptr_t                fd;
	struct taskdata         *task;	/*所属任务*/
	struct hostentry        *host;	/*如果不为空，则为该缓存用于通信的对端主机信息*/
	struct cache			cache;
};

struct procentry
{
	struct framentry        *frame;
	struct allcfg           *cfg;

	pthread_t               ptid;
	long                    tid;
	int                     idx;

	enum
	{
		PROC_STAT_STOP,
		PROC_STAT_RUN,
		PROC_STAT_PAUSE,
	}                       stat;

	enum
	{
		PROC_TYPE_EVENT,
		PROC_TYPE_CORO
	}                       type;

	/*which loop*/
	union
	{
		struct ev_loop          *evloop;
		struct evcoro_scheduler *corloop;
	};

	struct ev_async         ctrl;		// control to terminate the ev_loop
	int                     dealtasks;	// dealing task
	bool                    alive;		// alive task

	void                    *user;
};

struct framentry
{
	enum
	{
		FRAME_STAT_NONE,
		FRAME_STAT_INIT,
		FRAME_STAT_RUN,
		FRAME_STAT_STOP,
	}                       stat;
	// route data process
	int                     routeprocs;
	struct procentry        *routeproc;
	// receive data process
	struct procentry        recvproc;

	struct allcfg           *cfg;
	MemQueueT               queue;
};

struct taskdata
{
	struct procentry        *proc;
	struct allcfg           *cfg;

	struct hostentry        **rthost;	/*需要路由转发的主机信息*/
	int                     rthosts;

	struct netdata          src;		/*需要路由的源数据*/

	struct netdata          *caldata;	/*计算的结果数据*/
	struct netdata          *rtdata;	/*根据主机和协议拼接后的需要路由的数据*/
	int                     caldatas;
	int                     rtdatas;
	int						needcts;	// 需要计数的数量
	int                     acounter;	// 单线程，当前操作的以上数据数组计数器
};

struct  reqconn
{
	uint16_t        port;
	char            ipaddr[IPADDR_MAXSIZE];
};

/*
 *
 */
extern struct allcfg    g_allcfg;
extern struct framentry g_framentry;

void hostentry_destroy(struct hostentry *phost);

void hostgroup_destroy(struct hostgroup *grp);

void hostcluster_destroy(struct hostcluster *cluster);

void hostclusters_destroy(struct hostclusters *clusters);

void allcfg_destroy(struct allcfg *cfg);

bool hostgroup_connect(struct hostgroup *grp, int poolsize);

bool hostcluster_connect(struct hostcluster *cluster, int poolsize);

bool hostclusters_connect(struct hostclusters *clusters, int poolsize);

void netdata_destroy(struct netdata *data, bool distcnt);

void taskdata_free(struct taskdata **task, bool distcnt);

void routerule_destroy(struct routerule *rule);

void datasrc_destroy(struct datasrc *src);

void taghash_destroy(struct taghash *tag);

void procentry_stop(struct procentry *proc);
#endif	/* data_model_h */

