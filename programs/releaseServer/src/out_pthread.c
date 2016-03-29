/*
 *   ============================================================
 *   模块说明：
 *        此模块为单个接出处理线程的轮启框架
 *   使用要求：
 *        要求外部初始化OTHER_PTHREAD
 *        要求外部实现OTHER_PTHREAD中的init和data_handle
 *
 *   ============================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <ev.h>
#include <errno.h>

#include "out_pthread.h"
#include "cq_list.h"
#include "lock.h"
#include "log.h"

#if 0
static void async_cb(struct ev_loop *loop, ev_async *watcher, int event)
{
	OTHER_PTHREAD   *ptrd = (OTHER_PTHREAD *)watcher->data;
	void            *data = NULL;

	if (-1 == cqu_pull(ptrd->cq, &data)) {
		log_info(LOG_S, "队列为空\n");
		return;
	}

	assert(ptrd->data_handle);

	ptrd->data_handle(data);
	free(data);
}
#endif

static void io_rd_cb(struct ev_loop *loop, ev_io *watcher, int event)
{
	OTHER_PTHREAD   *ptrd = (OTHER_PTHREAD *)watcher->data;
	void            *data = NULL;
	char            yes = 0;
	int             ret = 0;

	ret = read(watcher->fd, &yes, sizeof(yes));

	if (ret <= 0) {
		if (errno == EINTR) {
			return;
		}

		log_info(LOG_SE, "read error:%s\n", strerror(errno));
		return;
	}

	if (1 != yes) {
		log_info(LOG_SE, "read value[%d] not 1\n", yes);
		return;
	}

	if (-1 == cqu_pull(ptrd->cq, &data)) {
		log_info(LOG_S, "队列为空\n");
		return;
	}

	assert(ptrd->data_handle);

	ptrd->data_handle(data);
	free(data);
}

static void heart_cb(struct ev_loop *loop, ev_timer *watcher, int event)
{
	OTHER_PTHREAD *ptrd = (OTHER_PTHREAD *)watcher->data;

	if (NULL == ptrd->do_heart) {
		log_info(LOG_S, "没有注册心跳函数,关闭该线程的心跳watcher\n");
		ev_timer_stop(loop, watcher);
		return;
	}

	if (-1 == ptrd->do_heart(ptrd->heart_space, ptrd->thread_idx)) {
		log_info(LOG_S, "心跳处理返回失败\n");
		return;
	}
}

/*
 *功能：pthread_create的回调函数
 * */
void *out_pthread_start(void *sync_wait)
{
	SYNC_WAIT *w = (SYNC_WAIT *)sync_wait;

	pthread_mutex_lock(&w->wait->mutex);

	WAIT            *wait;
	OTHER_PTHREAD   *thrd = NULL;

	wait = w->wait;
	/*获取线程属性*/
	thrd = (OTHER_PTHREAD *)w->data;

	/*日志初始化*/
	log_init(thrd->logfile);

	thrd->loop = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
	ev_io_init(&thrd->rd_watcher, io_rd_cb, thrd->pfd[0], EV_READ);
	ev_io_start(thrd->loop, &thrd->rd_watcher);
	thrd->rd_watcher.data = thrd;

	wait->count++;
	pthread_cond_signal(&wait->cond);
	pthread_mutex_unlock(&wait->mutex);

	/*libev async是不可靠信号,导致队列中的数据不能被处理*/

	/*
	 *        ev_async_init(&thrd->async_watcher,async_cb);
	 *        ev_async_start(thrd->loop,&thrd->async_watcher);
	 *        thrd->async_watcher.data = thrd;
	 */

	ev_timer_init(&thrd->heart_watcher, heart_cb, 3, thrd->heart_space);
	ev_timer_start(thrd->loop, &thrd->heart_watcher);
	thrd->heart_watcher.data = thrd;

	ev_run(thrd->loop, 0);

	return NULL;
}

