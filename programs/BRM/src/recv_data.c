//
//  recv_data.c
//  supex
//
//  Created by 周凯 on 15/10/23.
//  Copyright © 2015年 zk. All rights reserved.
//
#include <zmq.h>
#include "zmq_wrap.h"
#include "data_model.h"
#include "prg_frame.h"
#include "recv_data.h"

#define RCVDATA_STACK_SIZE (4096)

/*recv handle*/
struct zmqhandle
{
	void                    *ctx;		// context of zmq
	void                    *reqskt;	// socket for request
	void                    *rcvskt;	// socket for receive
	int                     reqfd;
	int                     rcvfd;
	char					reqname[32];
	char					rcvname[32];
	struct procentry        *proc;
	enum proto_type         proto;
	struct reqconn          reqdata;
};

static struct zmqhandle *_zmq_handle_new(struct hostgroup *grp, void *ctx);

static void _zmq_handle_free(struct zmqhandle *hdl);

static void _loop_idle(struct evcoro_scheduler *coro, struct procentry *proc);

static void _store_data(struct zmqhandle *hdl, struct zmqframe *frame);

static void _recv_data(struct evcoro_scheduler *scheduler, void *user);

/*发送请求让对端连接自己*/
static bool _send_request_cntme(struct evcoro_scheduler *scheduler, struct zmqhandle *hdl);

/* -----------------            */

/*
 * 解析接收的数据
 */
static struct taskdata  *_parse_data(struct allcfg *cfg, struct zmqframe *frame);

/*
 * 更具标签确定需要路由的主机组
 */
static struct hostentry **_get_route_host(struct allcfg *cfg, const char *tag, int *hosts);

/* -------------                */

void *recv_data(void *usr)
{
	struct procentry                *proc = usr;
	void *volatile                  zmqctx = NULL;
	
	assert(proc && proc->type == PROC_TYPE_CORO);

	pthread_cleanup_push((void (*)(void *))procentry_stop, proc);

	TRY
	{
		int                     i = 0;
		bool                    flag = false;
		struct  hostcluster     *hc = NULL; /**<source host pointer*/

		/* initialize */
		proc->tid = GetThreadID();

		/* wakeup and wait main thread*/
		futex_set_signal((int *)&proc->stat, PROC_STAT_RUN, 1);
		flag = futex_cond_wait((int *)&proc->frame->stat, FRAME_STAT_RUN, 10);
		if (unlikely(!flag)) ReturnValue(NULL);

		x_printf(D, "receive data thread `%ld` start.", proc->tid);

		/* start work */
		proc->corloop = evcoro_create(-1);
		AssertError(proc->corloop, ENOMEM);

		zmqctx = zmq_ctx_new();
		assert(zmqctx);
		/*add receive task to loop*/
		hc = &proc->cfg->host.srchost;
		
		for (i = 0; i < hc->hostgrps; i++) {
			struct zmqhandle *hdl = NULL;
			hdl = _zmq_handle_new(&hc->hostgrp[i], zmqctx);
			hdl->proc = proc;

			flag = evcoro_push(proc->corloop, _recv_data, hdl, RCVDATA_STACK_SIZE);

			if (unlikely(!flag)) {
				_zmq_handle_free(hdl);
				RAISE_SYS_ERROR_ERRNO(ENOMEM);
			}
		}

		evcoro_loop(proc->corloop, (evcoro_taskcb)_loop_idle, proc);
	}
	CATCH
	{}
	FINALLY
	{
		evcoro_destroy(proc->corloop, (evcoro_destroycb)_zmq_handle_free);
		while (zmqctx && (zmq_ctx_term(zmqctx) != 0)) {
			AssertRaise(errno == EINTR, EXCEPT_SYS);
		}
		x_printf(E, "receive data thread `%ld` end.", proc->tid);
	}
	END;

	pthread_cleanup_pop(true);

	return NULL;
}

static void _recv_data(struct evcoro_scheduler *scheduler, void *user)
{
	assert(user);
	struct zmqhandle        *hdl = user;
	int						rswitch = 0; /**< read number frame at least when it need to switch*/
	int                     pframes = 0;
	bool                    flag = false;
	int						*itm = &hdl->proc->cfg->idlesleep;
	int	volatile			counter = 0;
	struct zmqframe         *rcvframe = NULL; /**< receive multi-frame data from of zmq*/

	x_printf(D, "receive data start `%p`.", hdl);
	rswitch = MAX(hdl->proc->cfg->host.srchost.hostgrps, 1);
	rswitch = hdl->proc->cfg->queuesize / rswitch;
	rswitch = MAX(rswitch, 10);
	rcvframe = zmqframe_new(hdl->proc->cfg->pkgsize);
	return_if_fail(rcvframe);
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)zmqframe_free, rcvframe);

req_again:
	/*send request*/
	flag = _send_request_cntme(scheduler, hdl);

	if (unlikely(!flag)) {
		x_perror("write_zmqdata() from %s error : %s", hdl->reqname, x_strerror(errno));
		return;
	} else {
		union evcoro_event      event = {};
		// sleep .5s , switch to another task
		evcoro_timer_init(&event, ((float)*itm) / 1000);
		evcoro_idleswitch(scheduler, &event, EVCORO_TIMER);
	}
	// receive data
	do {
		UNREFOBJ(rcvframe);
		pframes = read_zmqdata(hdl->rcvskt, rcvframe);

		if (likely(pframes == 2)) {
			// store data
			_store_data(hdl, rcvframe);
			if (unlikely(! (counter++ % rswitch))) {
				// fast switch to another coroutine
				evcoro_fastswitch(scheduler);
			}
		} else if (likely(pframes == 0)) {
			union evcoro_event      revent = {};
			evcoro_io_init(&revent, hdl->rcvfd, ((float)*itm) / 100);
			flag = evcoro_idleswitch(scheduler, &revent, EVCORO_READ);
			// try again
			if (likely(flag)) continue; /*already read and continue*/
			else goto req_again; /*time out and send request again*/
		} else if (likely(pframes < 0)) {
			// a error occured on zmq
			x_perror("read_zmqdata() from %s error : %s", hdl->rcvname, x_strerror(errno));
			break;
		} else {
			// protocol error
			errno = EPROTO;
			x_perror("read_zmqdata() from %s error : %s", hdl->rcvname, x_strerror(EPROTO));
			break;
		}
	} while (1);

	evcoro_cleanup_pop(scheduler, true);

	x_printf(D, "end data start `%p`.", hdl);
}

static bool _send_request_cntme(struct evcoro_scheduler *scheduler, struct zmqhandle *hdl)
{
	bool                    ret = false;
	int                     pframes = 0;
	struct reqconn          *conn = NULL;
	struct zmqframe         *reqframe = NULL;
	union evcoro_event      event = {};

	/*allocate memory of frame*/
	reqframe = zmqframe_new(sizeof(*conn));
	return_val_if_fail(reqframe, false);
	evcoro_cleanup_push(scheduler, (evcoro_destroycb)zmqframe_free, reqframe);

	/*fill first frame data*/
	reqframe->frame[0] = sizeof(*conn);
	reqframe->frames = 1;
	conn = (struct reqconn *)&reqframe->data[0];
	memcpy(conn, &hdl->reqdata, sizeof(*conn));
	reqframe->offset += sizeof(*conn);
	REFOBJ(reqframe);

again:
	pframes = write_zmqdata(hdl->reqskt, reqframe);

	if (likely(pframes > 0)) ret = true;
	else if (likely(pframes == 0)) {
		evcoro_timer_init(&event, ((float)hdl->proc->cfg->idlesleep) / 1000);
		evcoro_idleswitch(scheduler, &event, EVCORO_TIMER);
		goto again;
	} else  ret = false;

	evcoro_cleanup_pop(scheduler, true);
	return ret;
}

static void _store_data(struct zmqhandle *hdl, struct zmqframe *frame)
{
	struct taskdata *volatile       data = NULL;
	bool                            flag = false;
	int                             esize = 0;
	MemQueueT                       mq = hdl->proc->frame->queue;
	SQueueT                         sq = mq->data;

	/*parse route tag to json*/
	data = _parse_data(hdl->proc->cfg, frame);
	return_if_fail(data);

	/*initialize*/
	data->proc = hdl->proc;

push_again:
	flag = MEM_QueuePush(mq, (char *)&data, sizeof(data), &esize);

	if (unlikely(!flag)) {
		int s = hdl->proc->cfg->idlesleep * 10;
		futex_wait(&sq->nodes, sq->capacity, s);
		goto push_again;
	} else if (likely(esize == sizeof(data))) {
		futex_wake(&sq->nodes, -1);
	} else {
		x_perror("no space to store data that receive by zmq!!!!!");
		abort();
	}
}

/*
 * 数据格式 路由标签数据json字符串 , 路由转发数据json字符串
 * {"src_name":["field1", "field2", ...]}'\1'{"..."}
 */
static struct taskdata *_parse_data(struct allcfg *cfg, struct zmqframe *frame)
{
	struct taskdata *volatile data = NULL;

	return_val_if_fail(frame->frame[0] > 0 && frame->frame[1] > 0, NULL);

	TRY
	{
		/*check formation of data*/
		New(data);
		AssertError(data, ENOMEM);

		/* parse route tag by first frame data*/
		char    *buff = frame->data;
		char    chr = buff[frame->frame[0]];

		buff[frame->frame[0]] = '\0';
		data->rthost = _get_route_host(cfg, buff, &data->rthosts);
		buff[frame->frame[0]] = chr;
		/* initialize data*/
		data->cfg = cfg;
		data->src.fd = -1;
		data->src.task = data;
		/*store route data*/
		bool flag = false;
		ssize_t size = 0;
		flag = cache_initial(&data->src.cache);
		AssertRaise(flag, EXCEPT_SYS);
		/*store second frame data*/
		size = cache_append(&data->src.cache, &buff[frame->frame[0]], frame->frame[1]);
		RAISE_SYS_ERROR(size);
		x_printf(I, "source data : %.*s", cache_data_length(&data->src.cache),
				 cache_data_address(&data->src.cache));
	}
	CATCH
	{
		// ignore exception and don't store data
		taskdata_free((struct taskdata **)&data, false);
	}
	END;

	return data;
}

/*
 * 根据标签确定需要路由的主机组
 */
static struct hostentry **_get_route_host(struct allcfg *cfg, const char *tag, int *hosts)
{
	struct hostentry **volatile     rhost = NULL;
	cJSON *volatile                 json = NULL;

	int rhosts = 0;

	assert(cfg && tag);

	TRY
	{
		struct datasrc          *datasrc = NULL;
		struct routerule        *rule = &cfg->routerule;
		int                     i = 0;
		cJSON                   *src = NULL;

		x_printf(I, "route tag : %s", tag);

		json = cJSON_Parse(tag);
		AssertRaise(json, EXCEPT_RCVDATA_FAIL);

		i = cJSON_GetArraySize(json);
		AssertRaise(i == 1, EXCEPT_RCVDATA_FAIL);

		src = cJSON_GetArrayItem(json, 0);
		AssertRaise(src && src->string && src->string[0] && src->type == cJSON_Array, EXCEPT_RCVDATA_FAIL);

		/* 在配置中查找数据源标签*/
		for (i = 0; i < rule->datasrcs; i++)
			if (strcasecmp(rule->datasrc[i].name, src->string) == 0) {
				datasrc = &rule->datasrc[i];
				break;
			}

		/*配置中没有匹配的数据源，或标签中没有主机组信息，不需要路由*/

		if (unlikely(!datasrc || (datasrc->taghashs < 1) || !datasrc->taghash)) {
			x_printf(W, "match tag of source : %s in group of route host is failure.", src->string);
			RAISE(EXCEPT_MATCHINFO_FAIL);
		}

		cJSON   *field = NULL;
		int     fields = 0;
		/* 将路由字段组成数组*/
		fields = cJSON_GetArraySize(src);
		/* 不需要路由该数据*/
		if (unlikely(fields < 1)) RAISE(EXCEPT_NOTNEED_ROTDATA);

		rhosts = datasrc->taghashs;
		NewArray0(rhosts, rhost);
		AssertError(rhost, ENOMEM);

		/*遍历需要路由的主机标签*/
		for (i = 0; i < rhosts; i++) {
			uint32_t        hash = 0;
			int             pos = 0;
			struct taghash  *tagh = &datasrc->taghash[i];

			/*
			 * 散列确定路由主机
			 */
#if 0
			/*目标主机的路由字段超出范围，不路由该条消息到该主机*/
			if (unlikely((tagh->tag < 0) || (tagh->tag > fields - 1))) continue;
			pos = tagh->tag;
#else
			pos = INRANGE(tagh->tag, 0, fields - 1);
#endif
			field = cJSON_GetArrayItem(src, pos);
			hash = tagh->hash(field->valuestring, -1);
			hash %= tagh->hostgrp->hosts;
			rhost[i] = &tagh->hostgrp->host[hash];
		}

		SET_POINTER(hosts, rhosts);
	}
	CATCH
	{
		Free(rhost);
		RERAISE;
	}
	FINALLY
	{
		cJSON_Delete(json);
	}
	END;

	return rhost;
}

static struct zmqhandle *_zmq_handle_new(struct hostgroup *grp, void *ctx)
{
	struct zmqhandle *volatile hdl = NULL;

	assert(grp);

	TRY
	{
		int     flag = 0;

		New(hdl);
		hdl->ctx = ctx;

		hdl->reqdata.port = htons((uint16_t)grp->host[0].port);
		snprintf(hdl->reqdata.ipaddr, sizeof(hdl->reqdata.ipaddr), "%s", grp->host[0].ip);

		hdl->proto = grp->host[0].proto;
		// local listener - receive data
		hdl->rcvskt = zmq_socket(ctx, ZMQ_PULL);
		AssertRaise(hdl->rcvskt, EXCEPT_SYS);

		/*允许排队消息数量*/
		set_zmqopt(hdl->rcvskt, ZMQ_RCVHWM, 100);
		set_zmqopt(hdl->rcvskt, ZMQ_LINGER, 0);

		snprintf(hdl->rcvname, sizeof(hdl->rcvname), "tcp://%s:%d", grp->host[0].ip, grp->host[0].port);

		flag = zmq_bind(hdl->rcvskt, hdl->rcvname);
		RAISE_SYS_ERROR(flag);

		// remote server - send request data
		hdl->reqskt = zmq_socket(ctx, ZMQ_PUSH);
		AssertRaise(hdl->reqskt, EXCEPT_SYS);

		set_zmqopt(hdl->reqskt, ZMQ_IMMEDIATE, 1);
		set_zmqopt(hdl->reqskt, ZMQ_LINGER, 0);

		snprintf(hdl->reqname, sizeof(hdl->reqname), "tcp://%s:%d", grp->host[1].ip, grp->host[1].port);

		flag = zmq_connect(hdl->reqskt, hdl->reqname);
		RAISE_SYS_ERROR(flag);

		hdl->reqfd = get_zmqopt(hdl->reqskt, ZMQ_FD);
		hdl->rcvfd = get_zmqopt(hdl->rcvskt, ZMQ_FD);
		
	}
	CATCH
	{
		_zmq_handle_free(hdl);
		RERAISE;
	}
	END;
	return hdl;
}

static void _zmq_handle_free(struct zmqhandle *hdl)
{
	return_if_fail(hdl);

	if (hdl->rcvskt) {
		zmq_unbind(hdl->rcvskt, hdl->rcvname);
		zmq_close(hdl->rcvskt);
	}

	if (hdl->reqskt) {
		zmq_disconnect(hdl->reqskt, hdl->reqname);
		zmq_close(hdl->reqskt);
	}

	Free(hdl);
}

static void _loop_idle(struct evcoro_scheduler *coro, struct procentry *proc)
{
	assert(proc);

	if (unlikely(proc->frame->stat != FRAME_STAT_RUN) || (evcoro_actives(proc->corloop) < 1))
		evcoro_stop(proc->corloop);
	else {
		SQueueT q = proc->frame->queue->data;

		if (q->nodes == q->capacity) {
			int f = futex_wait(&q->nodes, q->capacity, proc->cfg->idlesleep * 100);

			if (likely(f == 0))
				/*忽略不可用状态，快速开始下一轮切换*/
				// evcoro_fastswitch(coro);
				x_printf(W, "the space of queue is too small.");
		}
	}
}
