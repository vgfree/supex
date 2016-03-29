//
//  randmap_test.c
//  supex
//
//  Created by 周凯 on 15/10/29.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include "cJSON.h"
#include "ev.h"
#include "http.h"
#include "net_cache.h"

#include "libmini.h"

#define MAX_THREAD 2

char cal_host[10] = {};

struct clntdata
{
	int                     clnt;
	struct http_parse_info  parser;
	struct net_cache        cache;
	ev_io                   event;
	int                     stat;
};

static struct clntdata  *new_clntdata(int fd);

static void free_clntdata(struct clntdata **data);

static void accept_clnt(struct ev_loop *loop, ev_io *lsnt, int event);

static void read_clnt(struct ev_loop *loop, ev_io *clnt, int event);

static void write_clnt(struct ev_loop *loop, ev_io *clnt, int event);

void *accept_thread(void *arg)
{
	assert(arg);
	int fd = (int)(intptr_t)arg;

	struct ev_loop *loop_thread = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
	assert(loop_thread);

	TRY
	{
		ev_io lsnt = {};
		ev_io_init(&lsnt, accept_clnt, fd, EV_READ);
		ev_io_start(loop_thread, &lsnt);

		ev_loop(loop_thread, 0);
	}
	CATCH
	{}
	FINALLY
	{
		ev_loop_destroy(loop_thread);
	}
	END;
}

int main(int argc, char **argv)
{
	int             i, flag = 0;
	pthread_t       ac_thread[MAX_THREAD];

	//	SLogSetLevel(SLOG_E_LEVEL);
	SLogOpen(argv[0], SLOG_I_LEVEL);

	if (unlikely(argc != 3)) {
		x_perror("usage %s <ip/host> <port/service>", argv[0]);
		return EXIT_FAILURE;
	}

	TRY
	{
		int lstfd = 0;

		lstfd = TcpListen(argv[1], argv[2], NULL, NULL);
		RAISE_SYS_ERROR(lstfd);

		snprintf(cal_host, sizeof(cal_host), "%s", argv[2]);

		FD_Enable(lstfd, O_NONBLOCK);

		for (i = 0; i < MAX_THREAD; i++) {
			flag = pthread_create(&ac_thread[i], NULL, accept_thread, (void *)(intptr_t)lstfd);
			RAISE_SYS_ERROR_ERRNO(flag);
		}
	}
	CATCH
	{
		// ev_break(loop, EVBREAK_ALL);
	}
	FINALLY
	{
		for (i = 0; i < MAX_THREAD; i++) {
			if (ac_thread[i] > 0) {
				pthread_join(ac_thread[i], NULL);
			}
		}

		// ev_loop_destroy(loop);
	}
	END;

	return EXIT_SUCCESS;
}

static void accept_clnt(struct ev_loop *loop, ev_io *lsnt, int event)
{
	int volatile clntfd = -1;

	struct clntdata *volatile clntdata = NULL;

	TRY
	{
		struct sockaddr_storage addr = {};
		socklen_t               alen = sizeof(addr);

		clntfd = accept(lsnt->fd, (SA)&addr, &alen);
		RAISE_SYS_ERROR(clntfd);

		FD_Enable(clntfd, O_NONBLOCK);

		clntdata = new_clntdata(clntfd);

		cache_free(&clntdata->cache);
		http_parse_init(&clntdata->parser, &clntdata->cache.buf_addr, &clntdata->cache.get_size);
		ev_io_init(&clntdata->event, read_clnt, clntdata->clnt, EV_READ);
		ev_io_start(loop, &clntdata->event);
	}
	CATCH
	{
		if (clntfd > -1) {
			close(clntfd);
		}

		free_clntdata((struct clntdata **)&clntdata);
	}
	END;
}

static const char headf[] = "HTTP/1.1 %s\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:Keep-Alive\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n"
	"%s";
static const char bodyf[] =
	"{\"%s\" : \"%ld.%d\"}";

static const char *HTTP_STAT_INFO[] = {
	"200 OK",
	"400 Not Found",
	"500 Internal Server Error",
};

/* BKDR hash function */
unsigned int directhash(int key)
{
	unsigned int    hash = key;
	unsigned int    retval = 11;

	return 0;

	if (hash % retval == 5) {
		return 1;
	} else if (hash % retval == 9) {
		return 2;
	} else {
		return 0;
	}
}

static void read_clnt(struct ev_loop *loop, ev_io *clnt, int event)
{
	assert(clnt->data);
	struct clntdata *data = clnt->data;

	TRY
	{
		int             stat = 0;
		ssize_t         bytes = 0;
		unsigned int    pos = 0;

		bytes = net_recv(&data->cache, clnt->fd, &stat);

		if (likely(bytes > 0)) {
			AssertError(stat == 0, ENOMEM);

			if (unlikely(!http_parse_both(&data->parser))) {
				/*data not all,go on recive*/
				ReturnVoid();
			}

			if (unlikely(data->parser.hs.err != HPE_OK)) {
				RAISE_SYS_ERROR_ERRNO(EPROTO);
			}

			/*接收到完整的数据*/
			time_t  ss = 0;
			int     rc = 0;
			int     us = 0;
			int     len = 0;
			char    buff[1024] = {};
			char    body[256] = {};

			TM_GetTimeStamp(&ss, &us);

			pos = directhash(us);
			cache_free(&data->cache);

			len = snprintf(body, sizeof(body), bodyf, cal_host, ss, us);
			len = snprintf(buff, sizeof(buff), headf, HTTP_STAT_INFO[pos], len, body);

			cache_add(&data->cache, buff, len);

			ev_io_stop(loop, &data->event);
			ev_io_init(&data->event, write_clnt, data->clnt, EV_WRITE);
			ev_io_start(loop, &data->event);
		} else if (likely(bytes == 0)) {
			RAISE_SYS_ERROR_ERRNO(ECONNRESET);
		} else {
			if (likely(errno == EAGAIN)) {
				ReturnVoid();
			} else {
				RAISE(EXCEPT_SYS);
			}
		}
	}
	CATCH
	{
		ev_io_stop(loop, &data->event);
		free_clntdata(&data);
	}
	END;
}

static void write_clnt(struct ev_loop *loop, ev_io *clnt, int event)
{
	assert(clnt->data);
	struct clntdata *data = clnt->data;

	TRY
	{
		int     stat = 0;
		ssize_t bytes = 0;

		bytes = net_send(&data->cache, clnt->fd, &stat);

		if (likely(bytes == 0)) {
			if (unlikely(stat != 0)) {
				if (likely(errno == EAGAIN)) {
					ReturnVoid();
				} else {
					RAISE(EXCEPT_SYS);
				}
			}
		} else if (bytes > 0) {
			ReturnVoid();
		}

		cache_free(&data->cache);
		http_parse_init(&data->parser, &data->cache.buf_addr, &data->cache.get_size);
		ev_io_stop(loop, &data->event);
		ev_io_init(&data->event, read_clnt, data->clnt, EV_READ);
		ev_io_start(loop, &data->event);
	}
	CATCH
	{
		ev_io_stop(loop, &data->event);
		free_clntdata(&data);
	}
	END;
}

static struct clntdata *new_clntdata(int fd)
{
	assert(fd > -1);
	struct clntdata *volatile clntdata = NULL;

	TRY
	{
		New(clntdata);
		AssertError(clntdata, ENOMEM);

		cache_init(&clntdata->cache);

		clntdata->event.data = clntdata;
		clntdata->clnt = fd;
		clntdata->stat = 0;
	}
	CATCH
	{
		if (clntdata) {
			cache_free(&clntdata->cache);
		}

		Free(clntdata);
		RERAISE;
	}
	END;

	return clntdata;
}

static void free_clntdata(struct clntdata **data)
{
	assert(data);

	return_if_fail(*data);

	close((*data)->clnt);

	cache_free(&(*data)->cache);
	Free(*data);
}

