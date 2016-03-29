/*
 * a simple server use libev
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdint.h>
#include <sys/time.h>

#include "mfptp_evcb.h"
#include "mfptp_api.h"
#include "net_cache.h"
#include "mfptp_parser.h"
#include "mfptp_def.h"
#include "mfptp_callback.h"
#include "rbtree.h"
#include "basic_type.h"
#include "mfptp_users_rbtree.h"
#include "redis_status.h"
#include "major_def.h"

/* test */
static struct timeval g_dbg_time;

extern int                      G_WORKER_COUNTS;
extern MASTER_PTHREAD           g_master_pthread;
extern MFPTP_WORKER_PTHREAD     *g_worker_pthread;

extern struct mfptp_settings    g_mfptp_settings;
extern struct data_node         *g_data_pool;

static struct linger    g_quick_linger = {
	.l_onoff        = 1,
	.l_linger       = 0
};
static struct linger    g_delay_linger = {
	.l_onoff        = 1,
	.l_linger       = 1
};
rb_root                 user_tree = RB_ROOT;

/********************************************************/
static void _mfptp_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents);

static void _mfptp_recv_cb(struct ev_loop *loop, ev_io *w, int revents);

void _mfptp_send_cb(struct ev_loop *loop, ev_io *w, int revents);

/********************************************************/
static void mfptp_dispatch_task(struct user_info *p_info)
{
	unsigned int            idx = 0;
	int                     stp = 0;
	int                     mul = 1;
	MFPTP_WORKER_PTHREAD    *p_worker = NULL;

	/*dispath to a hander thread*/
	idx = 0;// mfptp_bkdr_hash( p_info->who ) % G_WORKER_COUNTS;
	p_worker = &g_worker_pthread[idx];
	p_info->processor = p_worker;
#ifdef USE_PIPE
DISPATCH_LOOP:

	if (write(p_worker->pfds[1], (char *)&p_info, sizeof(uintptr_t *)) != sizeof(uintptr_t *)) {
		if (0 == ((++stp) % (mul * SERVER_BUSY_ALARM_FACTOR))) {
			mul++;
			x_printf(S, "recv worker thread is too busy!\n");
			LOG(LOG_MFPTP_AUTH, S, "%s: recv worker thread is too busy!\n", p_info->who);
		}

		goto DISPATCH_LOOP;
	}

#else
	CQ_ITEM *p_item = &p_info->recv_item;

	cq_push(&(p_worker->qlist), p_item);
	ev_async_send(p_worker->loop, &(p_worker->async_watcher));	// TODO
#endif

	LOG(LOG_MFPTP_AUTH, D, "%s:master dispath task success!\n", p_info->who);
}

static void _mfptp_pass_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	x_printtime(&g_dbg_time);

	struct user_info *p_info = w->data;

	int ret = net_send(&p_info->send, w->fd, &p_info->control);

	if (ret >= 0) {
		if (ret > 0) {
			x_printf(D, "-> no all\n");
			LOG(LOG_MFPTP_AUTH, W, "%s:there is still data in send cache!\n", p_info->who);
			return;
		} else {
			x_printf(D, "-> is all\n");
			LOG(LOG_MFPTP_AUTH, I, "%s:there is no data in send cache!\n", p_info->who);
		}
	}

	cache_free(&p_info->send);
	p_info->control = X_DONE_OK;
	ev_io_stop(loop, w);
	int active = ev_is_active(w);

	LOG(LOG_MFPTP_AUTH, D, "authorize data send OK, now dispath task!\n");

	mfptp_dispatch_task(p_info);

	LOG(LOG_MFPTP_AUTH, M, "user %s authorize module OK\n\n", p_info->who);
	return;

P_BROKEN:
	cache_free(&p_info->send);
	ev_io_stop(loop, w);
	close(w->fd);
	free(p_info);
}

static void *callback_auth(void *data)
{
	struct user_info        *puser = (struct user_info *)data;
	struct mfptp_status     *p_status = &puser->mfptp_info.status;
	int                     cnt = p_status->package.frames;

	if ((REQ_METHOD == puser->mfptp_info.parser.method) && (1 == cnt)) {
		int len = MFPTP_UID_MAX_LEN;

		if (p_status->package.dsizes[0] < MFPTP_UID_MAX_LEN) {
			len = p_status->package.dsizes[0];
		}

		memcpy(puser->who, p_status->package.ptrs[0], len);
		puser->who[len] = 0;
	} else {
		memcpy(puser->who, MFPTP_INVALID_UID, MFPTP_INVALID_UID_LEN);
		puser->who[MFPTP_INVALID_UID_LEN] = 0;
	}

	return NULL;
}

static void _mfptp_auth_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	char                    temp[MAX_DEF_LEN] = { 0 };
	struct user_info        *p_info = w->data;
	int                     continution_index = 0;
	int                     continution_size = 0;
	int                     ret = x_net_recv(&p_info->recv, w->fd, &p_info->control);

	continution_size = p_info->recv.back_size;
	x_printf(D, "continution_size =%d\n", continution_size);
	x_printtime(&g_dbg_time);
	LOG(LOG_MFPTP_AUTH, M, "user  authorize module start!\n");
	ev_timer_stop(loop, &(p_info->alive_timer));

	if (ret > 0) {
		x_printf(D, "接收到的数据长度:%d\n", p_info->recv.get_size);
		LOG(LOG_MFPTP_AUTH, D, "recv data len in net cache:%d\n", p_info->recv.get_size);
		int i = 0;
		do {
			/*no more memory*/
			if (p_info->control == X_MALLOC_FAILED) {
				LOG(LOG_MFPTP_AUTH, E, "no more memory\n");
				goto A_BROKEN;
			}

			/*data too large*/
			if (p_info->control == X_DATA_TOO_LARGE) {
				LOG(LOG_MFPTP_AUTH, E, "data too large\n");
				goto A_BROKEN;
			}

			/* parse recive data */
			if (MFPTP_PARSE_OVER != mfptp_parse(p_info)) {	// FIXME
				/* 认证数据包不完成，继续接收数据*/
				LOG(LOG_MFPTP_AUTH, E, "data not all,go on recive!\n\n");
				return;
			} else {
				LOG(LOG_MFPTP_PARSE, M, "mfptp module parse over,user %s authorize data  parse over!\n\n", p_info->who);
				/* 协议分析状态初始化 */
				// mfptp_init_parser_info(&(p_info->mfptp_info));
			}

			p_info->auth_status = IS_AUTH;

			if (0 == strncmp(p_info->who, MFPTP_INVALID_UID, MFPTP_UID_MAX_LEN)) {
				/* 登陆失败 */
				LOG(LOG_MFPTP_AUTH, D, "user  %s  is illegal\n", p_info->who);
				/* add 刘金阳，用户标识不合发也要进行清理工作 */
				goto A_BROKEN;
			} else {
				/* 登陆成功 */
				ev_io_stop(loop, w);
				p_info->who[15] = 0;

				if (p_info->force_login == 1) {
					if (FALSE == rb_remove_user(&user_tree, p_info->who)) {
						x_printf(D, "rb_remove_user: %s failed\n", p_info->who);
					}
				}

				/* 插入到红黑树中*/
				if (rb_insert_user(&user_tree, p_info) == TRUE) {
					/* 插入成功 */
					mfptp_init_parser_info(&(p_info->mfptp_info));
					mfptp_register_callback(p_info, mfptp_drift_out_callback);
					continution_index = 0;
				} else {
					/* 插入失败, 表示该用户记录还未被删除，查找到该用户*/
					p_info = rb_search_user(&user_tree, p_info->who);

					/* 该用户的fd已经在新连接中从新生成, 需要从新设置*/
					p_info->old_sfd = p_info->sfd;
					x_printf(D, "old fd=%d, new fd=%d\n", p_info->sfd, w->fd);
					LOG(LOG_MFPTP_AUTH, D, "%s:old fd=%d, new fd=%d\n", p_info->who, p_info->sfd, w->fd);
					p_info->sfd = w->fd;
					p_info->auth_status = IS_AUTH;

					/* 不需要插入则需要加入内存释放 */
					ev_io_stop(loop, w);
					free(w->data);

					/* 改过移动数据后，续传数据计数更改 */
					continution_index = p_info->recv.back_size;
					continution_index += continution_size;
					p_info->recv.back_size += continution_size;
				}

				x_printf(D, "user %s  is online!\n", p_info->who);
				LOG(LOG_MFPTP_AUTH, D, "user %s  is online!\n", p_info->who);
				topo_set_user_data(p_info->who, (void *)p_info);

				/* 插入成功给用户回复OK */
				if (1) {
					printf("%s:continution_index=%d\n", p_info->who, continution_index);
				}

				LOG(LOG_MFPTP_AUTH, D, "%s:continution_index=%d\n", p_info->who, continution_index);
				int len = mfptp_pack_login_ok(temp, continution_index, p_info);
				x_printf(D, "user authorize OK %d\n\n", len);
				LOG(LOG_MFPTP_AUTH, D, " %s:send authorize data,len = %d \n", p_info->who, len);

				if (len > 0) {
					p_info->online = MFPTP_ONLINE;

					if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
						char    *id = p_info->who;
						int     ret1 = set_user_online(id, g_mfptp_settings.conf->file_info.redis_address, g_mfptp_settings.conf->file_info.redis_port);

						if (ret1 == 0) {
							x_printf(D, "save to redis success\n");
						} else {
							x_printf(D, "save to redis failed\n");
						}
					}

					/* 发送认证响应包给客户端 */
					int mem_ret = cache_add(&p_info->send, temp, len);

					if (mem_ret != X_DONE_OK) {
						LOG(LOG_MFPTP_AUTH, D, "%s:authorize data cache error! \n", p_info->who);
					}

					/* 停止读取认证数据的input watcher */
					// ev_io_stop(loop, &(p_info->i_watcher));

					ev_io *p_watcher = &(p_info->o_watcher);
					p_watcher->data = p_info;
					ev_io_init(p_watcher, _mfptp_pass_cb, p_info->sfd, EV_WRITE);
					ev_io_start(loop, p_watcher);
				} else {
					/* 异常处理*/
					p_info->auth_status = NO_AUTH;
					printf("who = %s\n", &p_info->who);
					char *id = p_info->who;
					p_info->online = MFPTP_OFFLINE;

					if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
						int ret2 = set_user_offline(id, g_mfptp_settings.conf->file_info.redis_address, g_mfptp_settings.conf->file_info.redis_port);

						if (ret2 == 0) {
							x_printf(D, "change to redis success\n");
						} else {
							x_printf(D, "change to redis fail\n");
						}
					}

					goto A_BROKEN;
				}
			}
		} while (0);
	} else if (ret == 0) {	/* socket has closed when read after */
		LOG(LOG_MFPTP_AUTH, D, "%s:remote socket closed!socket fd: %d\n", p_info->who, w->fd);
		x_printf(D, "11111 remote socket closed!socket fd: %d\n", w->fd);
		setsockopt(w->fd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
		goto A_BROKEN;
	} else {
		/* 此处read数据失败，ret必然小于零,根据errno来处理 */
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			/* 这两个值应该不会出现，做保护以防万一 */
			return;
		} else {/* socket is going to close when reading */
			x_printf(D, "ret :%d ,close socket fd : %d\n", ret, w->fd);
			LOG(LOG_MFPTP_AUTH, D, "%s:ret :%d ,close socket fd : %d\n", p_info->who, ret, w->fd);
			setsockopt(w->fd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
			goto A_BROKEN;
		}
	}

	return;

A_BROKEN:
	LOG(LOG_MFPTP_AUTH, M, "user %s auth broken!\n\n", p_info->who);
	cache_free(&p_info->recv);
	ev_io_stop(loop, w);
	close(w->fd);
	free(p_info);
}

void alive_timer_cb(EV_P_ ev_timer *w, int revents)
{
	struct user_info *p_info = (struct user_info *)w->data;

	if (!p_info) {
		return;
	}

	x_printf(D, "fd=%d,很久没有发起认证，关闭\n", p_info->sfd);
	LOG(LOG_MFPTP_AUTH, D, "fd=%d,很久没有发起认证，关闭\n", p_info->sfd);
	cache_free(&p_info->recv);
	cache_free(&p_info->send);
	ev_io_stop(loop, &p_info->i_watcher);
	ev_io_stop(loop, &p_info->o_watcher);
	close(p_info->sfd);
	free(p_info);
}

uint32_t get_timestamp()
{
	struct timeval  tv;
	uint32_t        millis = 0;

	if (gettimeofday(&tv, NULL)) {} else {
		millis = tv.tv_sec;
	}

	return millis;
}

void mfptp_check_online(EV_P_ ev_timer *w, int revents)
{
	struct   user_info      *p_info = (struct user_info *)w->data;
	uint32_t                now;

	if (!p_info) {
		return;
	}

	now = get_timestamp();

	LOG(LOG_TIMER, D, "now = %d  ,user =%s\n", now, p_info->who);
	LOG(LOG_TIMER, D, "p_info->last_coming=%d,user = %s\n", p_info->last_coming, p_info->who);

	if ((now - p_info->last_coming) > 40) {
		p_info->online = MFPTP_OFFLINE;

		if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
			char    *id = p_info->who;
			int     ret1 = set_user_offline(id, g_mfptp_settings.conf->file_info.redis_address, g_mfptp_settings.conf->file_info.redis_port);

			if (ret1 == 0) {
				x_printf(D, "change to redis success\n");
			} else {
				x_printf(D, "change to redis failed\n");
			}
		}

		p_info->online = MFPTP_OFFLINE;
		LOG(LOG_TIMER, D, "user=%s timeout offline,p_info->online = %d\n", p_info->who, p_info->online);
		ev_timer_stop(loop, w);
	} else {}
}

void mfptp_accept_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	int newfd;

	LOG(LOG_NET_CONN, M, "user connection module start!\n");
	/*accept*/
	struct sockaddr_in      sin;
	socklen_t               addrlen = sizeof(struct sockaddr);

	while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			/* these are transient, so don't log anything. */
			continue;
		} else {
			x_printf(D, "accept error.[%s]\n", strerror(errno));
			LOG(LOG_NET_CONN, E, "accept error.[%s]\n", strerror(errno));
			return;
		}
	}

	x_printf(D, "one connection, from %s:%d..\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
	LOG(LOG_NET_CONN, I, "one connection, from %s:%d..\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

	if (newfd >= MAX_LIMIT_FD) {
		x_printf(D, "error : this fd (%d) too large!\n", newfd);
		LOG(LOG_NET_CONN, D, "error : this fd (%d) too large!\n", newfd);
		send(newfd, FETCH_MAX_CNT_MSG, strlen(FETCH_MAX_CNT_MSG), 0);
		// setsockopt(newfd, SOL_SOCKET, SO_LINGER, (const char *)&g_delay_linger, sizeof(g_delay_linger));
		close(newfd);
		return;
	}	// FIXME how to message admin

	x_printtime(&g_dbg_time);
	/*set status*/
	fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK);
	x_printf(D, "----===one connection, from %s:%d..\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

	/*set the new connect*/
	struct user_info *p_info = calloc(1, sizeof(struct user_info));
	p_info->mfptp_info.status.doptr = 0;
	mfptp_init_parser_info(&(p_info->mfptp_info));

	if (p_info) {
		cache_init(&p_info->recv);
		cache_init(&p_info->send);

		p_info->sfd = newfd;
		p_info->old_sfd = -1;
		p_info->port = ntohs(sin.sin_port);
		mfptp_set_user_secret_key(p_info->key);
		inet_ntop(AF_INET, &sin.sin_addr, p_info->szAddr, INET_ADDRSTRLEN);
		p_info->auth_status = NO_AUTH;
		p_info->online = MFPTP_OFFLINE;
		x_printf(D, "timer start\n");
		p_info->alive_timer.data = p_info;
		ev_timer_init(&(p_info->alive_timer), alive_timer_cb, 150., 0);
		ev_timer_start(loop, &(p_info->alive_timer));
		ev_io *p_watcher = &(p_info->i_watcher);
		p_watcher->data = p_info;
		mfptp_register_callback(p_info, mfptp_auth_callback);
		ev_io_init(p_watcher, _mfptp_auth_cb, newfd, EV_READ);
		ev_io_start(loop, p_watcher);
		LOG(LOG_NET_CONN, M, "user connection module OK, next, authorize the user!\n\n");
	} else {
		// TODO
	}
}

static int fetch_link_work_task(MFPTP_WORKER_PTHREAD *p_worker, struct ev_loop *loop)
{
	CQ_ITEM *item = cq_pop(&(p_worker->qlist));

	if (item != NULL) {
		struct user_info        *p_info = item->data;
		ev_io                   *p_watcher = &(p_info->i_watcher);
		p_watcher->data = p_info;
		ev_io_init(p_watcher, _mfptp_recv_cb, p_info->sfd, EV_READ);
		ev_io_start(loop, p_watcher);

		p_info->last_coming = get_timestamp();
		p_info->online = MFPTP_ONLINE;
		ev_timer *p_timer = &(p_info->online_timer);
		ev_timer_stop(loop, p_timer);
		ev_timer_init(p_timer, mfptp_check_online, 80, 80.);
		ev_timer_start(loop, p_timer);

		return 1;
	}

	return 0;
}

#ifdef USE_PIPE
static int fetch_pipe_work_task(MFPTP_WORKER_PTHREAD *p_worker, struct ev_loop *loop)
{
	struct user_info        *p_info = NULL;
	int                     n = read(p_worker->pfds[0], (char *)&p_info, sizeof(uintptr_t *));

	assert((n == -1) || (n == sizeof(uintptr_t *)));

	if (n == sizeof(uintptr_t *)) {
		p_info->online = MFPTP_ONLINE;

		if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
			x_printf(D, "status timer start....\n");
			p_info->last_coming = get_timestamp();
			ev_timer *p_timer = &(p_info->online_timer);
			p_timer->data = p_info;
			ev_timer_stop(loop, p_timer);
			ev_timer_init(p_timer, mfptp_check_online, 15, 60.);
			ev_timer_start(loop, p_timer);
		}

		ev_io *p_watcher = &(p_info->i_watcher);

		if (MFPTP_INVALID_FD != p_info->old_sfd) {
			x_printf(D, "stop relogin watcher reader----------------------------------------------\n");
			ev_io_stop(loop, p_watcher);
		}

		p_watcher->data = p_info;
		ev_io_init(p_watcher, _mfptp_recv_cb, p_info->sfd, EV_READ);
		ev_io_start(loop, p_watcher);
		return 0;
	}

	return 1;
}

static int fetch_weibo_pipe_work_task(MFPTP_WORKER_PTHREAD *p_worker, struct ev_loop *loop)
{
	struct user_info        *p_info = NULL;
	int                     n = read(p_worker->weibo_pfds[0], (char *)&p_info, sizeof(uintptr_t *));

	if (n == sizeof(uintptr_t *)) {
		ev_io *p_watcher = &(p_info->o_watcher);
		p_watcher->data = p_info;
		int active = ev_is_active(p_watcher);

		if (!active) {
			ev_io_init(p_watcher, _mfptp_send_cb, p_info->sfd, EV_WRITE);
			ev_io_start(loop, p_watcher);
		}

		return 0;
	}

	return 1;
}

  #define MAX_TASK_FETCH_DEPTH 25
void mfptp_fetch_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)(w->data);
	int                     i = 0;

	while (!!fetch_pipe_work_task(p_worker, loop) && (++i <= MAX_TASK_FETCH_DEPTH)) {}
}

void mfptp_weibo_fetch_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)(w->data);
	int                     i = 0;

	while (!!fetch_weibo_pipe_work_task(p_worker, loop) && (++i <= MAX_TASK_FETCH_DEPTH)) {}
}

void mfptp_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	int                     idx = 0;
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)(w->data);

	while (!!fetch_link_work_task(p_worker, loop)) {
		idx++;
	}

	if (idx == 0) {
		fetch_pipe_work_task(p_worker, loop);
	}
}

#else
void mfptp_async_cb(struct ev_loop *loop, ev_async *w, int revents)
{
	MFPTP_WORKER_PTHREAD *p_worker = (MFPTP_WORKER_PTHREAD *)(w->data);

	fetch_link_work_task(p_worker, loop);
}

void mfptp_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	MFPTP_WORKER_PTHREAD *p_worker = (MFPTP_WORKER_PTHREAD *)(w->data);

	while (!!fetch_link_work_task(p_worker, loop)) {}
}
#endif	/* ifdef USE_PIPE */

static void _mfptp_recv_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	struct user_info        *p_info = w->data;
	int                     parse_ret;
	int                     ret = x_net_recv(&p_info->recv, w->fd, &p_info->control);

	LOG(LOG_MFPTP_PARSE, D, "11p_info->last_coming= %d,user = %s,p_info->online =%d,status = %d \n", p_info->last_coming, p_info->who, p_info->online, g_mfptp_settings.conf->file_info.user_online_status);
	x_printtime(&g_dbg_time);

	if (ret > 0) {
		if (p_info->control < X_DONE_OK) {
			goto R_BROKEN;
		}

		p_info->last_coming = get_timestamp();
		LOG(LOG_MFPTP_PARSE, D, "p_info->last_coming= %d,user = %s,p_info->online =%d,status = %d \n", p_info->last_coming, p_info->who, p_info->online, g_mfptp_settings.conf->file_info.user_online_status);

		if (MFPTP_OFFLINE == p_info->online) {
			// if (1) {
			LOG(LOG_MFPTP_PARSE, D, "%s:...set online too \n", p_info->who);
			p_info->online = MFPTP_ONLINE;

			if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
				char    *id = p_info->who;
				int     ret1 = set_user_online(id, g_mfptp_settings.conf->file_info.redis_address, g_mfptp_settings.conf->file_info.redis_port);

				if (ret1 == 0) {
					x_printf(D, "save to redis success\n");
				} else {
					x_printf(D, "save to redis failed\n");
				}

				ev_timer_stop(loop, &(p_info->online_timer));
				ev_timer *p_timer = &(p_info->online_timer);
				p_timer->data = p_info;
				ev_timer_init(p_timer, mfptp_check_online, 15, 60.);
				ev_timer_start(loop, &(p_info->online_timer));
			}
		} else {}

		LOG(LOG_MFPTP_PARSE, D, "%s:_mfptp_recv_cb %d -- received data\n", p_info->who, ret);
		x_printf(D, "_mfptp_recv_cb %d -- received data\n", ret);
		/* parse recive data */
		parse_ret = mfptp_parse(p_info);

		if (MFPTP_PARSE_OVER == parse_ret) {
			x_printf(D, "_mfptp_recv_cb -- received data parse over\n");
			LOG(LOG_MFPTP_PARSE, M, "%s:mfptp parse module OK, parse one request data over！\n\n", p_info->who);

			mfptp_init_parser_info(&(p_info->mfptp_info));
			mfptp_register_callback(p_info, mfptp_drift_out_callback);
		}

		if (p_info->control == X_PARSE_ERROR) {
			goto R_BROKEN;
		}
	} else if (ret == 0) {	/* socket has closed when read after */
		LOG(LOG_MFPTP_PARSE, D, "remote socket closed!socket fd: %d\n", w->fd);
		x_printf(D, " %s: remote socket closed!socket fd: %d\n", p_info->who, w->fd);
		setsockopt(w->fd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
		goto R_BROKEN;
	} else {
		x_printf(D, "ret aaaaaaaaaaaaaaaaaaaaaaaaaaaaa:%d %u \n", ret, errno);

		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			x_printf(D, "ret aaaaaaaaaaaaaaaaaaaaaaaaaaaaa:%d \n", ret);
			return;
		} else {/* socket is going to close when reading */
			LOG(LOG_MFPTP_PARSE, D, "%s:ret :%d ,close socket fd : %d\n", p_info->who, ret, w->fd);
			x_printf(D, "ret :%d ,close socket fd : %d\n", ret, w->fd);
			setsockopt(w->fd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
			goto R_BROKEN;
		}
	}

	return;

R_BROKEN:
	switch (p_info->control)
	{
		LOG(LOG_MFPTP_PARSE, M, "%s:mfptp parse module failed\n\n", p_info->who);

		case X_MALLOC_FAILED:
			LOG(LOG_MFPTP_PARSE, D, "%s:NO MORE MEMORY!\n", p_info->who);
			x_printf(D, "no more memory!\n");
			break;

		case X_DATA_TOO_LARGE:
			LOG(LOG_MFPTP_PARSE, D, "%s:DATA TOO LARGE!\n", p_info->who);
			x_printf(D, "data too large!\n");
			break;
	}
	// clean rbtree
	LOG(LOG_MFPTP_PARSE, D, "now,user %s: is offline!\n\n", p_info->who);
	x_printf(D, "now ,user %s: is offline\n", p_info->who);
	// rb_remove_user(&user_tree, p_info->who);
	// cache_free( &p_info->recv );//TODO clean all
	// cache_free( &p_info->send );//TODO clean all
	// close(w->fd);
	p_info->auth_status = NO_AUTH;
	char *id = p_info->who;
	p_info->online = MFPTP_OFFLINE;

	if (g_mfptp_settings.conf->file_info.user_online_status == 1) {
		int ret1 = set_user_offline(id, g_mfptp_settings.conf->file_info.redis_address, g_mfptp_settings.conf->file_info.redis_port);

		if (ret1 == 0) {
			x_printf(D, "change to redis success:%s\n", p_info->who);
		} else {
			x_printf(D, "change to redis failed:%s\n", p_info->who);
		}
	}

	ev_io_stop(loop, &p_info->i_watcher);
	ev_io_stop(loop, &p_info->o_watcher);

	// free(p_info);
}

void _mfptp_send_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	x_printtime(&g_dbg_time);
	struct user_info *p_info = w->data;

	int ret = net_send(&p_info->send, w->fd, &p_info->control);

	if (ret >= 0) {
		if (ret > 0) {
			x_printf(D, "-> no all\n");
			LOG(LOG_NET_DOWNLINK, D, "there is still data in send cache\n");
			// TODO 数据前移
			return;
		} else {// send return -1
			x_printf(D, "-> is all\n");
			LOG(LOG_NET_DOWNLINK, D, "there is no data in send cache\n");

			if (p_info->control != X_DONE_OK) {
				goto S_BROKEN;
			}
		}
	}

	cache_free(&p_info->send);
	p_info->control = X_DONE_OK;
	ev_io_stop(loop, w);
	// fprintf(stderr,"stop stop stop stop stop\n");
	LOG(LOG_NET_DOWNLINK, M, "downlink module OK\n\n");
	return;

S_BROKEN:
	// clean rbtree
	// rb_remove_user(&user_tree, p_info->who);
	// cache_free( &p_info->recv );//TODO clean all
	// cache_free( &p_info->send );//TODO clean all
	// close(w->fd);
	ev_io_stop(loop, &p_info->i_watcher);
	ev_io_stop(loop, &p_info->o_watcher);
	LOG(LOG_NET_DOWNLINK, M, "down link module failed\n\n");

	// free(p_info);
}

static void _mfptp_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents)
{}

void mfptp_update_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	int space = get_overplus_time();

	if (0 == space) {
		x_printf(S, "update ...\n");

		/*reget timer*/
		space = get_overplus_time();
	}

	/* reset timer */
	ev_timer_stop(loop, w);
	ev_timer_init(w, mfptp_update_cb, space, 0.);
	ev_timer_start(loop, w);
}

extern struct file_log_s g_file_log;	// 日志结构
void mfptp_signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
	x_printf(D, "get a signal\n");

	if (w->signum == SIGQUIT) {
		// TODO free pool buff
		ev_signal_stop(loop, w);
		ev_break(loop, EVBREAK_ALL);
	} else if (w->signum == SIGUSR1) {	// 处理SIGUSR1信号，add by 刘金阳
		g_file_log.change_log = 1;
	}
}

