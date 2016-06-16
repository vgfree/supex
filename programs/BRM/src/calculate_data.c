#include "pool_api.h"
#include "async_api.h"
#include "route_data.h"
#include "calculate_data.h"
#include "cache/cache.h"

static void _calculate_fail(const struct async_obj *obj, void *data);

static void _calculate_all_finish(const struct async_obj *obj, void *data);

static void _calculate_one_finish(struct async_obj *obj, void *reply, void *data);

/**
 * 更具主机信息和源数据信息拼接发送数据
 * @param src 源数据
 * @param host 计算主机信息
 * @param calreq 计算请求数据
 */
static void _calculate_make_data(struct netdata *src, struct hostentry *host, struct netdata *calreq);

void start_calculate_data(struct taskdata *data)
{
	assert(data && cache_data_address(&data->src.cache) &&
		cache_data_length(&data->src.cache));

	/* protocal parse*/
	struct async_ctx *volatile ac = NULL;
	/*calculate hosts*/
	struct allcfg           *cfg = data->cfg;
	bool                    ischeckfd = cfg->ischeckfd;
	struct hostgroup        *calgrp = &cfg->host.calhost;
	volatile int            i = 0;

	TRY
	{
		ac = async_initial(data->proc->evloop, QUEUE_TYPE_CORO,
				_calculate_fail, _calculate_all_finish, data, 0);
		AssertError(ac, ENOMEM);

		data->acounter = 0;
		data->needcts = 0;
		data->caldatas = calgrp->hosts;
		NewArray0(data->caldatas, data->caldata);
		AssertError(data->caldata, ENOMEM);

		/* send to all of calculator hosts*/
		for (i = 0; i < data->caldatas; i++) {
			/* send data to calculator host*/
			AssertError(calgrp->host[i].errconn == 0, EAGAIN);
			data->caldata[i].fd = -1;
			data->caldata[i].task = data;
			data->caldata[i].proto = calgrp->host[i].proto;
			data->caldata[i].host = &calgrp->host[i];
			/*gain and store data to netdata*/
			int rc = 0;
			rc = conn_xpool_gain(&data->caldata[i].cntpool,
					calgrp->host[i].ip,
					calgrp->host[i].port,
					(void **)&data->caldata[i].fd);
			AssertError(rc == POOL_API_OK, ECONNREFUSED);

			/*保证描述符没有被对端关闭*/
			while (unlikely(ischeckfd && SF_IsClosed((int)data->caldata[i].fd))) {
				conn_xpool_free(data->caldata[i].cntpool, (void **)&data->caldata[i].fd);
				rc = conn_xpool_pull(data->caldata[i].cntpool,
						(void **)&data->caldata[i].fd);
				AssertError(rc == POOL_API_OK, ECONNREFUSED);
			}

			/*make data*/
			_calculate_make_data(&data->src, &calgrp->host[i], &data->caldata[i]);

			/*add to command for send data to remote calculate host*/
			async_command(ac, data->caldata[i].proto,
				(int)data->caldata[i].fd,
				data->caldata[i].cntpool,
				_calculate_one_finish, &data->caldata[i],
				cache_data_address(&data->caldata[i].cache),
				cache_data_length(&data->caldata[i].cache));
			data->needcts++;
		}

		if (likely(data->needcts > 0)) {
			/*成功启动一个任务，增加正在处理的任务数*/
			async_startup(ac);
			data->proc->dealtasks++;
			x_printf(D, "start calculate data by `%p`", data);
		} else {
			x_printf(W, "calculate data fail by `%p`!!!", data);
			async_distory(ac);
			/*可以暂存在一个临时队列中*/
			taskdata_free((struct taskdata **)&data, false);
		}
	}
	CATCH
	{
		const ExceptT *e = GetExcept();

		if ((e == &EXCEPT_SYS) && (errno == ECONNREFUSED)) {
			/*连接失败计数*/
			calgrp->host[i].errconn++;
			x_printf(E, "can't connect to (%s:%d) or its pool of connection is full",
				calgrp->host[i].ip, calgrp->host[i].port);
		}

		x_printf(W, "calculate data fail by `%p`!!!", data);

		if (unlikely(ac)) {
			async_distory(ac);
		}

		RERAISE;
	}
	END;
}

static const char       CALDATA_URL[] = "/publicentry";
static const char       CALDATA_TEMPLATE[] =
	"POST %s HTTP/1.1\r\n"
	"User-Agent: curl/7.33.0\r\n"
	"Host: %s:%d\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:Keep-Alive\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n";

static void _calculate_make_data(struct netdata *src, struct hostentry *host, struct netdata *calreq)
{
	ssize_t size = 0;
	bool    flag = false;

	flag = cache_initial(&calreq->cache);
	AssertRaise(flag, EXCEPT_SYS);

	/*根据协议组装数据*/
	if (likely(calreq->proto == PROTO_TYPE_HTTP)) {
		size = cache_appendf(&calreq->cache, CALDATA_TEMPLATE,
				host->url ? host->url : CALDATA_URL,
				host->ip, host->port,
				cache_data_length(&src->cache));
		RAISE_SYS_ERROR(size);

		size = cache_append(&calreq->cache,
				cache_data_address(&src->cache),
				cache_data_length(&src->cache));
		RAISE_SYS_ERROR(size);
	} else {
		RAISE_SYS_ERROR_ERRNO(ENOPROTOOPT);
	}

	calreq->host = host;
	x_printf(I, "calculate data : %.*s",
		cache_data_length(&calreq->cache),
		cache_data_address(&calreq->cache));
}

static void _calculate_fail(const struct async_obj *obj, void *usr)
{
	struct async_ctx        *ctx = usr;
	struct taskdata         *data = ctx->data;

	assert(data);

	x_printf(W, "calculate data fail!!!");
	data->proc->dealtasks--;
	/*send data fail and disconnect connection*/
	taskdata_free(&data, false);
}

static void _calculate_all_finish(const struct async_obj *obj, void *usr)
{
	struct async_ctx                *ctx = usr;
	struct taskdata *volatile       data = ctx->data;

	assert(data);

	TRY
	{
		AssertRaise(data->acounter == data->needcts,
			EXCEPT_CALDATA_FAIL);

		/*next work*/
		data->proc->dealtasks--;
		start_route_data(data);
	}
	CATCH
	{
		taskdata_free((struct taskdata **)&data, false);
	}
	END;
}

static void _calculate_one_finish(struct async_obj *obj, void *reply, void *usr)
{
	struct netdata *data = usr;

	assert(data);
	struct taskdata *task = data->task;
	assert(task);

	/*如果其中一个计算主机没有返回数据*/

	TRY
	{
		if (likely(reply)) {
			int                     size = 0;
			const char              *body = NULL;
			struct net_cache        *cache = reply;

			if (unlikely(cache->get_size == 0)) {
				x_printf(W, "no data");
				ReturnVoid();
			}

			/*store information about protocol-parser*/
			if (likely(data->proto == PROTO_TYPE_HTTP)) {
				int status = 0;

				status = obj->replies.work->parse.http_info.hs.status_code;
				size = (int)obj->replies.work->parse.http_info.hs.body_size;

				if (unlikely((status != 200) || (size < 1))) {
					x_printf(W, "host %s:%d calculate fail : %.*s",
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
				body = &cache->buf_addr[obj->replies.work->parse.http_info.hs.body_offset];
			} else {
				RAISE(EXCEPT_ROTDATA_FAIL);
			}

			/* reused cache and store data*/
			cache_clean(&data->cache);
			ssize_t ret = 0;
			ret = cache_append(&data->cache, body, size);
			RAISE_SYS_ERROR(ret);
			task->acounter++;

			x_printf(I, "host %s:%d calculate data result : %.*s",
				data->host->ip, data->host->port,
				cache_data_length(&data->cache),
				cache_data_address(&data->cache));
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

