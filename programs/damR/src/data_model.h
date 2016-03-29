//
//  data_model.h
//  supex
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef data_model_h
#define data_model_h

#include <stdio.h>
#include "evcoro_scheduler.h"
#include "ev.h"
#include "cJSON.h"
#include "libmini.h"

__BEGIN_DECLS
/* --------             */
typedef bool (*queue_pushpull_cb)(void *queue, char *buff, int size, int *effectsize);
typedef void *(*queue_create_cb)(const char *path, int nodetotal, int nodesize);
typedef void (*queue_destroy_cb)(void **queue, bool delmtdata, NodeDestroyCB destroy);

struct queue
{
	int                     magic;
	queue_pushpull_cb       push;
	queue_pushpull_cb       priopush;
	queue_pushpull_cb       pull;

	queue_create_cb         create;
	queue_destroy_cb        destroy;

	int                     capacity;
	int                     cellsize;

	char                    name[MAX_PATH_SIZE];
	void                    *queue;

	char                    *userspace;
	int                     *nodes;
};
/* -------------------			*/
struct netcfg
{
	char    *ip;
	int     port;
	enum
	{
		PROTO_ZMQ,
		PROTO_HTTP
	}       proto;
	int     cache;
};

struct queuecfg
{
	enum
	{
		FILE_QUEUE = 1,
		MEM_QUEUE,
		SHM_QUEUE
	}       type;
	char    *path;
	int     capacity;
	int     cellsize;
	int     seq;
};

struct logcfg
{
	char    *path;
	int     level;
};

struct cfg
{
	int             magic;
	struct netcfg   recv;
	struct netcfg   send;
	struct queuecfg queue;
	struct logcfg   log;
	char            *cfgfile;
};
/* -------------------			*/

struct iohandle
{
	char    name[32];
	enum
	{
		SOCK_ZMQ_BIND = 1,
		SOCK_ZMQ_CONN,
		SOCK_HTTP_BIND,
		SOCK_HTTP_CONN
	}       type;
	union
	{
		struct
		{
			void    *ctx;
			void    *skt;
			int     fd;
		}       zmq;
		int     http;
	};

	void    *usr;
	/*延迟关闭时间*/
	int     linger;
};
/* -------------------			*/
struct frame;
struct proc
{
	int             magic;
	pthread_t       ptid;
	long            tid;
	struct frame    *frame;

	union
	{
		struct evcoro_scheduler *coroloop;
		struct ev_loop          *evloop;
	};

	enum
	{
		PROC_STAT_INIT = 1,
		PROC_STAT_RUN,
		PROC_STAT_STOP,
		PROC_STAT_PAUSE
	}               stat;
};

struct frame
{
	int             magic;
	struct cfg      *cfg;
	struct queue    *queue;
	enum
	{
		FRAME_STAT_INIT = 1,
		FRAME_STAT_RUN,
		FRAME_STAT_STOP
	}               stat;

	//	struct iohandle *rcvlstn;
	//	struct iohandle *sndlstn;

	struct proc     rcvproc;
	struct proc     sndproc;
};
/* -------------------			*/
struct  reqconn
{
	uint16_t        port;
	char            ipaddr[IPADDR_MAXSIZE];
};
/* -------------------			*/
extern struct queue     g_queue;
extern struct cfg       g_cfg;
extern struct frame     g_frame;

void stop_proc(struct proc *proc);

void kill_proc(struct proc *proc);

void iohandle_destroy(struct iohandle *io);

__END_DECLS
#endif	/* data_model_h */

