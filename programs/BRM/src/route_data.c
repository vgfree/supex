/*****************************************************************************
 * Copyright(c) shanghai... 2015-2100
 * Filename: route_data.c
 * Author: shaozhenyu 18217258834@163.com
 * History:
 *        created by shaozhenyu 2015-10-23
 * Description:
 *
 ******************************************************************************/
#include "ev.h"
#include "pool_api/conn_xpool_api.h"
#include "async_tasks/async_api.h"
#include "route_data.h"
#include "calculate_data.h"
#include "cache/cache.h"

#if TEST
AO_T g_TestCnt = 0;
#endif

static void _route_fail(const struct async_obj *obj, void *data);

static void _route_all_finish(const struct async_obj *obj, void *data);

static void _route_one_finish(struct async_obj *obj, void *reply, void *data);

static void _route_pause(struct ev_loop *loop, ev_async *async, int event);

static void _route_idle(struct ev_loop *loop, ev_idle *idle, int event);

static void _route_make_rtdata(struct taskdata *data);

void *route_data(void *usr)
{
	struct procentry *proc = usr;

	assert(proc && proc->type == PROC_TYPE_EVENT);

	pthread_cleanup_push((void (*)(void *))procentry_stop, proc);

	TRY
	{
		bool flag = false;

		/* initialize */
		proc->tid = GetThreadID();

		BindCPUCore(-1);

		/* wait main thread*/
		futex_set_signal((int *)&proc->stat, PROC_STAT_RUN, 1);
		flag = futex_cond_wait((int *)&proc->frame->stat, FRAME_STAT_RUN, 10);

		if (unlikely(!flag)) {
			ReturnValue(NULL);
		}

		x_printf(D, "route thread `%ld` start.", proc->tid);

		ev_idle idle = {};

		ev_idle_init(&idle, _route_idle);
		ev_idle_start(proc->evloop, &idle);

		ev_async_init(&proc->ctrl, _route_pause);
		ev_async_start(proc->evloop, &proc->ctrl);

		ev_set_userdata(proc->evloop, proc);
		ev_loop(proc->evloop, 0);
	}
	CATCH
	{}
	FINALLY
	{
		x_printf(E, "route thread `%ld` end.", proc->tid);
	}
	END;

	pthread_cleanup_pop(true);

	return NULL;
}

void start_route_data(struct taskdata *data)
{
	assert(data);

	struct allcfg                   *cfg = data->cfg;
	bool                            ischeckfd = cfg->ischeckfd;
	struct async_ctx *volatile      ac = NULL;
	volatile int                    i = 0;
	TRY
	{
		_route_make_rtdata(data);
		data->acounter = 0;
		data->needcts = 0;

		ac = async_initial(data->proc->evloop, QUEUE_TYPE_CORO,
				_route_fail, _route_all_finish, data, 0);
		AssertError(ac, ENOMEM);

		for (i = 0; i < data->rthosts; i++) {
			if (unlikely(!data->rtdata[i].host ||
				/*有失败的情况*/
				(data->rtdata[i].host->errconn != 0))) {
				continue;
			}

			/*gain and store data to netdata*/
			int rc = 0;
			rc = conn_xpool_gain(&data->rtdata[i].cntpool,
					data->rtdata[i].host->ip,
					data->rtdata[i].host->port,
					(void **)&data->rtdata[i].fd);

			/*保证描述符没有被对端关闭*/
			if (unlikely(ischeckfd && (rc == POOL_API_OK) &&
				SF_IsClosed((int)data->rtdata[i].fd))) {
check_fd:
				conn_xpool_free(data->rtdata[i].cntpool,
					(void **)&data->rtdata[i].fd);
				rc = conn_xpool_pull(data->rtdata[i].cntpool,
						(void **)&data->rtdata[i].fd);
			}

			if (likely(rc == POOL_API_OK)) {
				if (unlikely(ischeckfd &&
					SF_IsClosed((int)data->rtdata[i].fd))) {
					goto check_fd;
				}
			} else {
				if (unlikely(rc == POOL_API_ERR_OP_FAIL)) {
					AO_INC(&data->rtdata[i].host->errconn);
					x_printf(E, "can't connect %s:%d",
						data->rtdata[i].host->ip,
						data->rtdata[i].host->port);
				} else {
					x_printf(W, "the pool of connection (%s:%d) is full",
						data->rtdata[i].host->ip,
						data->rtdata[i].host->port);
				}

				continue;
			}

			async_command(ac, data->rtdata[i].proto,
				(int)data->rtdata[i].fd,
				data->rtdata[i].cntpool,
				_route_one_finish, &data->rtdata[i],
				cache_data_address(&data->rtdata[i].cache),
				cache_data_length(&data->rtdata[i].cache));
			data->needcts++;
		}

		if (likely(data->needcts > 0)) {
			async_startup(ac);
			data->proc->dealtasks++;
			x_printf(D, "start route data by `%p`", data);
		} else {
			x_printf(W, "route data fail by `%p`!!!", data);
			async_distory(ac);
			taskdata_free((struct taskdata **)&data, false);
		}
	}
	CATCH
	{
		if (errno == ECONNREFUSED) {
			/*连接失败计数*/
		}

		x_printf(W, "route data fail by `%p`!!!", data);

		if (unlikely(ac)) {
			async_distory(ac);
		}

		RERAISE;
	}
	END;
}

static void _route_idle(struct ev_loop *loop, ev_idle *idle, int event)
{
	struct procentry                *proc = ev_userdata(loop);
	struct taskdata *volatile       data = NULL;
	MemQueueT                       mqueue = NULL;
	SQueueT                         squeue = NULL;
	struct allcfg                   *cfg = NULL;
	bool                            flag = false;

	assert(proc);
	cfg = proc->cfg;
	assert(cfg);

	TRY
	{
		mqueue = proc->frame->queue;
		squeue = mqueue->data;

		// x_printf(D, "get route data idle ...");

		if (proc->frame->stat != FRAME_STAT_RUN) {
			ev_idle_stop(loop, idle);
			ev_break(loop, EVBREAK_ALL);
			ReturnVoid();
		}

		/* 是否已达到最大处理数*/
		if (unlikely(proc->dealtasks >= cfg->paralleltasks)) {
			ReturnVoid();
		}

		/* pull source data from queue*/
		flag = MEM_QueuePull(mqueue, (char *)&data, sizeof(data), NULL);

		if (unlikely(!flag)) {
			/*wait a moment*/
			futex_wait(&squeue->nodes, 0, cfg->idlesleep * 100);
			ReturnVoid();
		} else {
			futex_wake(&squeue->nodes, -1);
		}

		/* modify owner of data*/
		assert(data);
		data->proc = proc;

		/* 决定是否需要中间计算过程*/
		if (cfg->calculate) {
			start_calculate_data(data);
		} else {
			start_route_data(data);
		}
	}
	CATCH
	{
		taskdata_free((struct taskdata **)&data, false);
	}
	END;
}

static void _route_pause(struct ev_loop *loop, ev_async *async, int event)
{
	struct procentry *proc = ev_userdata(loop);

	assert(proc);
	assert(async->data);

	x_printf(D, "wait all of task finish to suspend route thread ...");

	if (likely(proc->dealtasks > 0)) {
		ev_async_send(loop, async);
	} else {
		x_printf(D, "suspend route thread ...");
		/*挂起线程*/
		ThreadSuspendStart(async->data);

		x_printf(D, "restart route thread ...");
	}
}

static const char RTDATA_URL[] = "/publicentry";

static const char RTDATA_TEMPLATE[] =
	"POST %s HTTP/1.1\r\n"
	"User-Agent: curl/7.33.0\r\n"
	"Host: %s:%d\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:Keep-Alive\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n";

static void _route_make_rtdata(struct taskdata *task)
{
	struct cache json = {};

	AssertError(task->rthost && task->rthosts > 0, EINVAL);

	TRY
	{
		/*确定组合所有待发送数据的长度*/
		/*组合所有待发送数据为一个json*/
		int             i = 0;
		bool            flag = false;
		ssize_t         size = 0;
		const char      *start = NULL;
		const char      *end = NULL;

		flag = cache_initial(&json);
		AssertError(flag, ENOMEM);

		size = cache_append(&json, "{", 1);
		RAISE_SYS_ERROR(size);

		/*仅发送json数据*/
		/*原数据*/
		if (likely(cache_data_length(&task->src.cache) > 0)) {
			start = x_strchr(cache_data_address(&task->src.cache),
					cache_data_length(&task->src.cache), '{');
			AssertRaise(start, EXCEPT_RCVDATA_FAIL);
			end = x_strrchr(cache_data_address(&task->src.cache),
					cache_data_length(&task->src.cache), '}');
			AssertRaise(end && start + 1 < end, EXCEPT_RCVDATA_FAIL);
			cache_appendf(&json, "%.*s", (int)(end - start - 1), start + 1);
		}

		/*计算数据*/
		for (i = 0; i < task->caldatas; i++) {
			start = x_strchr(cache_data_address(&task->caldata[i].cache),
					cache_data_length(&task->caldata[i].cache), '{');
			AssertRaise(start, EXCEPT_RCVDATA_FAIL);
			end = x_strrchr(cache_data_address(&task->caldata[i].cache),
					cache_data_length(&task->caldata[i].cache), '}');
			AssertRaise(end && start + 1 < end, EXCEPT_RCVDATA_FAIL);
			size = cache_appendf(&json, ",%.*s", (int)(end - start - 1), start + 1);
			RAISE_SYS_ERROR(size);
		}

		size = cache_append(&json, "}", 1);
		RAISE_SYS_ERROR(size);

		x_printf(I, "body of route data : %.*s",
			cache_data_length(&json),
			cache_data_address(&json));

		task->rtdatas = task->rthosts;
		NewArray0(task->rtdatas, task->rtdata);
		AssertError(task->rtdata, ENOMEM);

		for (i = 0; i < task->rthosts; i++) {
			/*初始化路由缓冲信息*/
			struct hostentry *host = task->rthost[i];

			if (unlikely(!host)) {
				continue;
			}

			task->rtdata[i].fd = -1;
			task->rtdata[i].task = task;
			task->rtdata[i].host = host;
			task->rtdata[i].proto = host->proto;

			flag = cache_initial(&task->rtdata[i].cache);
			AssertRaise(flag, EXCEPT_SYS);

			if (likely(host->proto == PROTO_TYPE_HTTP)) {
				size = cache_appendf(&task->rtdata[i].cache, RTDATA_TEMPLATE,
						host->url ? host->url : RTDATA_URL,
						host->ip, host->port,
						cache_data_length(&json));
				RAISE_SYS_ERROR(size);
				size = cache_append(&task->rtdata[i].cache,
						cache_data_address(&json),
						cache_data_length(&json));

				x_printf(I, "route data : %.*s",
					cache_data_length(&task->rtdata[i].cache),
					cache_data_address(&task->rtdata[i].cache));
			} else {
				RAISE_SYS_ERROR_ERRNO(ENOPROTOOPT);
			}
		}
	}
	FINALLY
	{
		cache_clean(&json);
		cache_finally(&json);
	}
	END;
}

static void _route_fail(const struct async_obj *obj, void *usr)
{
	assert(usr);
	struct async_ctx                *ctx = usr;
	struct taskdata *volatile       data = ctx->data;

	TRY
	{
#if TEST
		long cur = AO_F_ADD(&g_TestCnt, 1);

		if (unlikely(cur >= TEST)) {
			kill(0, SIGINT);
		}
#endif
		RAISE(EXCEPT_ROTDATA_FAIL);
	}
	CATCH
	{}
	FINALLY
	{
		data->proc->dealtasks--;
		taskdata_free((struct taskdata **)&data, false);
	}
	END;
}

static void _route_all_finish(const struct async_obj *obj, void *usr)
{
	assert(usr);
	struct async_ctx                *ctx = usr;
	struct taskdata *volatile       data = ctx->data;

	TRY
	{
#if TEST
		long cur = AO_F_ADD(&g_TestCnt, 1);

		if (unlikely(cur >= TEST)) {
			kill(0, SIGINT);
		}
#endif
		/*有失败的路由*/
		AssertRaise(data->acounter == data->needcts,
			EXCEPT_ROTDATA_FAIL);
	}
	CATCH
	{}
	FINALLY
	{
		data->proc->dealtasks--;
		taskdata_free((struct taskdata **)&data, false);
	}
	END;
}

static void _route_one_finish(struct async_obj *obj, void *reply, void *usr)
{
	struct netdata *data = usr;

	assert(data);
	struct taskdata *task = data->task;
	assert(task);

	TRY
	{
		if (likely(reply)) {
			struct net_cache *cache = reply;

			if (unlikely(cache->get_size == 0)) {
				x_printf(W, "host %s:%d route fail : no response",
					data->host->ip, data->host->port);
				ReturnVoid();
			}

			/*store information about protocol-parser*/
			if (likely(data->proto == PROTO_TYPE_HTTP)) {
				int status = 0;

				status = obj->replies.work->parse.http_info.hs.status_code;

				if (unlikely(status != 200)) {
					x_printf(W, "host %s:%d route fail : %.*s",
						data->host->ip, data->host->port,
						cache->get_size, cache->buf_addr);
					RAISE(EXCEPT_ROTDATA_FAIL);
				}

#if 0
				int keepalive = 0;
				keepalive = obj->replies.work->parse.http_info.hs.keep_alive;

				if (unlikely(!keepalive)) {
					/*close connection*/
					conn_xpool_free(data->cntpool, (void **)&data->fd);
				}
#endif
			} else {
				RAISE(EXCEPT_ROTDATA_FAIL);
			}

			x_printf(I, "host %s:%d route success : %.*s",
				data->host->ip, data->host->port,
				cache->get_size, cache->buf_addr);

			/*如果其中一个计算主机没有返回数据*/
			task->acounter++;
		}
	}
	CATCH
	{
		//				close((int)data->fd);
	}
	FINALLY
	{
		data->fd = -1;
		data->cntpool = NULL;
	}
	END;
}

