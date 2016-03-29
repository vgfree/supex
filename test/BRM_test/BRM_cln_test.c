/*******************************************************************/

/********************** Author: xuli *******************************/
/********************** Date: 2015/11/06 ***************************/

/*
 *   this file is for practice, create a virtual client of BRM to
 *   communicate with BRM and see how things going !
 */

/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"
#include "evcoro_scheduler.h"
#include "cache.h"
#include "net.h"

#include "libmini.h"

#define         CLNDATA_STAT_UNINIT     0
#define         CLNDATA_STAT_INIT       1
#define         CLNDATA_STAT_RUNNING    2
#define         CLNDATA_STAT_FINSHED    3

#define         STACK_SIZE              20 * 1024
#define         RECV_BUF_SIZE           2 * 1024
#define         HASH_MOD                5

struct clndata
{
	int                     fd;
	int                     stat;
	int                     coroid;
	struct http_parse_info  parser;
	struct netdata_cache    cache;
	struct evcoro_scheduler *corloop;
};

struct evcoro_scheduler *g_evcorloop = NULL;

static const char bodyf[] =
	"{%s |\"%d\" : \"%ld.%d\"}";

static const char headf[] = "HTTP/1.1 %s\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:Keep-Alive\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n"
	"%s";

static const char *HTTP_STAT_INFO[] = {
	"200 OK",
	"400 Not Found",
	"413 Request Entity Too Large",
	"500 Internal Server Error",
	"503 Service Unavailable"
};

static void _accept(struct evcoro_scheduler *corloop, void *user);

static void _recv_data(struct evcoro_scheduler *corloop, void *user);

static void _send_data(struct evcoro_scheduler *corloop, void *user);

static struct clndata   *_new_accept_clndata(struct evcoro_scheduler *corloop, int fd);

static void _initialize(struct clndata *clndate, const char *ip, const char *port);

static void _destroy_clndata(struct clndata *clndata);

static void _make_send_data(struct clndata *clndata);

static inline void init_signal(int number, ...);

static unsigned int BKDRhash(char *key);

int main(int argc, char **argv)
{
	if (argc < 3) {
		x_perror("usage : %s [ip] [port]", argv[0]);
		return -1;
	}

	struct clndata *volatile clndata = NULL;

	TRY
	{
		SLogOpen(argv[0], SLOG_D_LEVEL);
		_new(clndata);
		AssertError(clndata, ENOMEM);

		g_evcorloop = evcoro_create(-1);
		clndata->corloop = g_evcorloop;
		clndata->stat = CLNDATA_STAT_UNINIT;
		clndata->coroid = 0;

		_initialize(clndata, argv[1], argv[2]);
		init_signal(2, SIGINT, SIGQUIT);

		assert(clndata->stat);

		evcoro_push(clndata->corloop, _accept, clndata, STACK_SIZE);
		evcoro_loop(clndata->corloop, NULL, NULL);
	}
	FINALLY
	{
		evcoro_destroy(clndata->corloop, NULL);
		x_printf(D, "destroyed anything in main function");
	}
	END;

	return 0;
}

static void _accept(struct evcoro_scheduler *corloop, void *user)
{
	struct clndata *volatile        acpt_clndata = NULL;
	struct clndata *volatile        lsn_clndata = user;

	TRY
	{
		bool                    rc = false;
		int                     fd = -1;
		int                     coroid = 1;
		union evcoro_event      revent = {};

		AssertError(lsn_clndata->stat, EINVAL);
		evcoro_io_init(&revent, lsn_clndata->fd, 15);
		evcoro_cleanup_push(g_evcorloop, (evcoro_destroycb)_destroy_clndata, lsn_clndata);

		while (1) {
			fd = accept(lsn_clndata->fd, NULL, NULL);

			if (unlikely(fd < 0)) {
				if (likely((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))) {
					evcoro_idleswitch(corloop, &revent, EVCORO_READ);
					continue;
				} else {
					RAISE_SYS_ERROR(fd);
				}
			}

			x_printf(D, "\x1B[1;31m" "accepted socket fd: %d" "\x1B[m", fd);

			acpt_clndata = _new_accept_clndata(lsn_clndata->corloop, fd);
			acpt_clndata->coroid = coroid;
			coroid++;
			AssertError(acpt_clndata->stat == CLNDATA_STAT_INIT, EINVAL);
			acpt_clndata->stat = CLNDATA_STAT_RUNNING;

			x_printf(D, "push coro id:%d", acpt_clndata->coroid);
			rc = evcoro_push(acpt_clndata->corloop, _recv_data, (struct clndata *)acpt_clndata, STACK_SIZE);
			AssertRaise(rc, EXCEPT_SYS);

			evcoro_fastswitch(corloop);
		}
	}
	CATCH
	{
		x_perror("accept failed:%d", errno);
		evcoro_cleanup_pop(g_evcorloop, true);
	}
	END;

	evcoro_cleanup_pop(g_evcorloop, true);
}

static void _recv_data(struct evcoro_scheduler *corloop, void *user)
{
	struct clndata *volatile clndata = user;

	TRY
	{
		ssize_t                 bytes = 0;
		bool                    rc = false;
		char                    buf[RECV_BUF_SIZE] = { 0 };
		union evcoro_event      revent = {};

		evcoro_io_init(&revent, clndata->fd, 15);
		AssertError(clndata->stat == CLNDATA_STAT_RUNNING, EINVAL);
		evcoro_cleanup_push(corloop, (evcoro_destroycb)_destroy_clndata, clndata);

		while (1) {
			bytes = recv(clndata->fd, (void *)buf, sizeof(buf), 0);

			if (likely(bytes > 0)) {
				x_printf(D, "\x1B[1;31m" "recevie data" "\x1B[m");
				x_printf(D, "\n%s\n", (char *)buf);
				rc = netdata_cache_add(&clndata->cache, buf, (int)bytes);
				AssertRaise(rc, EXCEPT_ASSERT);

				if (unlikely(!http_parse_request(&clndata->parser))) {
					evcoro_fastswitch(corloop);
					continue;
				}

				if (unlikely(clndata->parser.hs.err != HPE_OK)) {
					RAISE_SYS_ERROR_ERRNO(EPROTO);
				}

				_make_send_data(clndata);
				rc = evcoro_push(clndata->corloop, _send_data, (struct clndata *)clndata, STACK_SIZE);
				AssertRaise(rc, EXCEPT_SYS);
				evcoro_fastswitch(corloop);
				break;
			} else if (bytes == 0) {
				x_printf(D, "\x1B[1;31m" "recevied data successful,socket fd:%d exit" "\x1B[m", clndata->fd);
				clndata->stat = CLNDATA_STAT_FINSHED;
				break;
			} else {
				if (likely((errno == EAGAIN) || (errno == EINTR) || (errno == EWOULDBLOCK))) {
					evcoro_idleswitch(corloop, &revent, EVCORO_READ);
					x_printf(D, "\x1B[1;31m" "RECV_DATA BLOCK" "\x1B[m");
					continue;
				} else {
					RAISE_SYS_ERROR(bytes);
				}
			}
		}
	}
	CATCH
	{
		x_perror("\x1B[1;31m" "recevie data failed:%d" "\x1B[m", errno);
		evcoro_cleanup_pop(corloop, true);
	}
	FINALLY
	{
		if (clndata->stat == CLNDATA_STAT_FINSHED) {
			evcoro_cleanup_pop(corloop, true);
		} else {
			evcoro_cleanup_pop(corloop, false);
		}
	}
	END;
}

static void _send_data(struct evcoro_scheduler *corloop, void *user)
{
	struct clndata *volatile clndata = user;

	TRY
	{
		ssize_t                 bytes = 0;
		bool                    rc = false;
		union evcoro_event      wevent = {};

		AssertError(clndata->stat == CLNDATA_STAT_RUNNING, EINVAL);
		evcoro_io_init(&wevent, clndata->fd, 10);

		while (1) {
			if (likely(clndata->cache.offset - clndata->cache.outsize > 0)) {
				bytes = send(clndata->fd, clndata->cache.data_buf + clndata->cache.outsize, clndata->cache.offset - clndata->cache.outsize, 0);

				if (likely(bytes > 0)) {
					x_printf(D, "\x1B[1;31m" "send data" "\x1B[m");
					x_printf(D, "\n%s", clndata->cache.data_buf + clndata->cache.outsize);
					clndata->cache.outsize += (int)bytes;

					if (unlikely(clndata->cache.offset > clndata->cache.outsize)) {
						// there still have data need to send in cache
						evcoro_fastswitch(corloop);
						continue;
					} else {
						netdata_cache_reset(&clndata->cache);
					}

					if (clndata->parser.hs.keep_alive) {
						rc = evcoro_push(clndata->corloop, _recv_data, (struct clndata *)clndata, STACK_SIZE);
						AssertRaise(rc, EXCEPT_SYS);
						evcoro_fastswitch(corloop);
					}

					break;
				} else {
					if (likely((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))) {
						x_printf(D, "\x1B[1;31m" "SEND_DATA BLOCK" "\x1B[m");
						evcoro_idleswitch(corloop, &wevent, EVCORO_WRITE);
						continue;
					} else {
						RAISE_SYS_ERROR(bytes);
					}
				}
			}
		}
	}
	CATCH
	{
		x_perror("send data failed");
		_destroy_clndata(clndata);
	}
	END;
}

static struct clndata *_new_accept_clndata(struct evcoro_scheduler *corloop, int fd)
{
	struct clndata *volatile clndata = NULL;

	_new(clndata);
	AssertError(clndata, ENOMEM);

	netdata_cache_init(&clndata->cache);
	http_parse_init(&clndata->parser, &clndata->cache.data_buf, (size_t *)&clndata->cache.offset);

	clndata->corloop = corloop;
	clndata->fd = fd;
	unblock(clndata->fd);

	clndata->stat = CLNDATA_STAT_INIT;

	return clndata;
}

static void _initialize(struct clndata *clndata, const char *ip, const char *port)
{
	int sockfd = -1;

	netdata_cache_init(&clndata->cache);
	http_parse_init(&clndata->parser, &clndata->cache.data_buf, (size_t)&clndata->cache.offset);

	sockfd = tcp_listen(ip, port);
	RAISE_SYS_ERROR(sockfd);

	clndata->fd = sockfd;
	unblock(clndata->fd);

	clndata->stat = CLNDATA_STAT_INIT;
}

static void _destroy_clndata(struct clndata *clndata)
{
	return_if_fail(clndata);

	x_printf(D, "I am here to free clndata,coro id;%d", clndata->coroid);

	if (clndata->fd > 0) {
		close(clndata->fd);
		clndata->fd = -1;
	}

	netdata_cache_free(&clndata->cache);
	_free(clndata);
}

static void _make_send_data(struct clndata *clndata)
{
	time_t          ss = 0;
	int             us = 0;
	int             len = 0;
	char            buff[1024] = {};
	char            body[256] = {};
	bool            rc = 0;
	char            url[128] = { 0 };
	unsigned int    pos = 0;

	pos = BKDRhash(clndata->cache.data_buf);
	TM_GetTimeStamp(&ss, &us);
	strncpy(url, clndata->cache.data_buf + clndata->parser.hs.url_offset, clndata->parser.hs.url_len);
	netdata_cache_reset(&clndata->cache);

	if (strcasestr(url, "driview")) {
		len = snprintf(body, sizeof(body), bodyf, "driview", getpid(), ss, us);
	} else if (strcasestr(url, "goby")) {
		len = snprintf(body, sizeof(body), bodyf, "goby", getpid(), ss, us);
	} else if (strcasestr(url, "ptop")) {
		len = snprintf(body, sizeof(body), bodyf, "ptop", getpid(), ss, us);
	} else if (strcasestr(url, "gopath")) {
		len = snprintf(body, sizeof(body), bodyf, "gopath", getpid(), ss, us);
	} else {
		x_perror("Bad URL");
		RAISE_SYS_ERROR(NULL);
	}

	len = snprintf(buff, sizeof(buff), headf, HTTP_STAT_INFO[pos], len, body);
	x_printf(D, "URL:%s", url);

	rc = netdata_cache_add(&clndata->cache, buff, len);
	AssertRaise(rc, EXCEPT_ASSERT);
}

static unsigned int BKDRhash(char *key)
{
	unsigned int    seed = 131;
	unsigned int    hash = 0;

	while (*key != 0) {
		hash = hash * seed + (*key++);
	}

	return hash % HASH_MOD;
}

static void sigdispose(int signum, siginfo_t *siginfo, void *usr)
{
	switch (signum)
	{
		case SIGINT:
			x_printf(D, "receive signal:SIGINT, the cause is:%d", siginfo->si_code);
			evcoro_stop(g_evcorloop);
			break;

		case SIGQUIT:
			x_printf(D, "receive signal:SIGQUIT,the cause is:%d", siginfo->si_code);
			evcoro_stop(g_evcorloop);
			break;

		default:
			x_printf(D, "break");
	}
}

static inline void init_signal(int number, ...)
{
	va_list ap;
	int     i = 0;

	struct sigaction action = {};

	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = sigdispose;
	sigemptyset(&action.sa_mask);

	va_start(ap, number);

	for (i = 0; i < number; i++) {
		if (sigaction(va_arg(ap, int), &action, NULL)) {
			RAISE_SYS_ERROR(-1);
		}
	}

	va_end(ap);
}

