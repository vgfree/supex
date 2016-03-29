/**************************************************************************
 *   File Name : libev_client.c
 *   Author    : shaozhenyu
 *   Mail      : 18217258834@163.com
 *   Created   : 2015年11月05日 星期四 11时03分41秒
 ************************************************************************/
#include <stdio.h>
#include "ev.h"
#include "http.h"
#include "net_cache.h"
#include "libmini.h"
//#include "http_parser.h"
#include "cJSON.h"

#define HASH_MOD        3
#define CAL_HOST_1      "10003"
#define CAL_HOST_2      "10004"

struct client_data
{
	int                     fd;
	int                     code;
	struct net_cache        cache;
	struct http_parse_info  parser;
	ev_io                   event;
};

struct times
{
	time_t  sec;
	int     us;
};

static void get_diff_time(const char *data, const time_t now_sec, const int now_us);

static struct client_data       *new_client_data(int fd);

static void free_client_data(struct client_data **data);

static void io_accept(struct ev_loop *loop, ev_io *io_w, int event);

static void io_read(struct ev_loop *loop, ev_io *io_w, int event);

static void io_write(struct ev_loop *loop, ev_io *io_w, int event);

static const char headf[] = "HTTP/1.1 %s\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:Keep-Alive\r\n"
	"Accept: */*\r\n\r\n";

static const char *HTTP_STAT_INFO[] = {
	"200 OK",
	"400 Not Found",
	"500 Internal Server Error",
};

/* BKDR hash function */

/*
 *   unsigned int BKDRhash(char *key)
 *   {
 *      unsigned int seed = 131;
 *      unsigned int hash = 0;
 *      unsigned int retval =  11;
 *
 *      while (*key != 0)
 *          hash = hash * seed + (*key++);
 *
 *      if (hash % retval ==  5)
 *              return 1;
 *      else if (hash % retval == 9)
 *              return 2;
 *      else
 *              return 0;
 *
 *      //return hash % HASH_MOD;
 *   }
 */
unsigned int directhash(int key)
{
	int     hash = key;
	int     retval = 11;

	if (hash % retval == 5) {
		return 1;
	} else if (hash % retval == 9) {
		return 2;
	} else {
		return 0;
	}
}

static void io_read(struct ev_loop *loop, ev_io *io_w, int event)
{
	assert(io_w->data);
	struct client_data *data = io_w->data;

	TRY
	{
		int             state = 0;
		unsigned int    pos = 0;
		ssize_t         bytes = 0;

		bytes = net_recv(&data->cache, io_w->fd, &state);

		if (likely(bytes > 0)) {
			AssertError(state == 0, ENOMEM);

			if (unlikely(!http_parse_request(&data->parser))) {
				// pos = BKDRhash(data->cache.buf_addr);
				ReturnVoid();
			}

			if (unlikely(data->parser.hs.err != HPE_OK)) {
				// pos = BKDRhash(data->cache.buf_addr);
				RAISE_SYS_ERROR_ERRNO(EPROTO);
			}

			time_t  sec = 0;
			int     us = 0;
			int     len = 0;
			char    buff[1024] = {};

			TM_GetTimeStamp(&sec, &us);
			// pos = BKDRhash(data->cache.buf_addr);
			pos = directhash(us);

			get_diff_time(data->cache.buf_addr, sec, us);
			cache_free(&data->cache);

			data->code = pos == 0 ? 200 : -1;
			len = snprintf(buff, sizeof(buff), headf, HTTP_STAT_INFO[pos]);

			cache_add(&data->cache, buff, len);

			ev_io_stop(loop, &data->event);
			ev_io_init(&data->event, io_write, data->fd, EV_WRITE);
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
		free_client_data(&data);
	}
	END;
}

static void io_write(struct ev_loop *loop, ev_io *io_w, int event)
{
	assert(io_w->data);
	struct client_data *data = io_w->data;

	TRY
	{
		int     state = 0;
		ssize_t bytes = 0;

		bytes = net_send(&data->cache, io_w->fd, &state);

		if (likely(bytes == 0)) {
			if (unlikely(state != 0)) {
				if (likely(errno == EAGAIN)) {
					ReturnVoid();
				} else {
					RAISE(EXCEPT_SYS);
				}
			}
		} else if (bytes > 0) {
			ReturnVoid();
		}

		assert(data->code == 200);

		cache_free(&data->cache);
		http_parse_init(&data->parser, &data->cache.buf_addr, &data->cache.get_size);

		ev_io_stop(loop, &data->event);
		ev_io_init(&data->event, io_read, data->fd, EV_READ);
		ev_io_start(loop, &data->event);
	}
	CATCH
	{
		ev_io_stop(loop, &data->event);
		free_client_data(&data);
	}
	END;
}

static void io_accept(struct ev_loop *loop, ev_io *io_w, int event)
{
	int                             client_fd = -1;
	struct client_data *volatile    client_data = NULL;

	TRY
	{
		socklen_t               addr_len = 0;
		struct sockaddr_in      addr = {};

		addr_len = sizeof(addr);
		client_fd = accept(io_w->fd, (SA)&addr, &addr_len);
		RAISE_SYS_ERROR(client_fd);

		FD_Enable(client_fd, O_NONBLOCK);

		client_data = new_client_data(client_fd);

		cache_free(&client_data->cache);
		http_parse_init(&client_data->parser, &client_data->cache.buf_addr, &client_data->cache.get_size);

		ev_io_init(&client_data->event, io_read, client_data->fd, EV_READ);
		ev_io_start(loop, &client_data->event);
	}
	CATCH
	{
		if (client_fd > -1) {
			close(client_fd);
		}

		free_client_data((struct client_data **)&client_data);
	}
	END;
}

int main(int argc, char *argv[])
{
	int             sfd = 0;
	struct ev_loop  *loop = NULL;
	ev_io           io_w = {};

	//	SLogSetLevel(SLOG_I_LEVEL);
	SLogOpen(argv[0], SLOG_I_LEVEL);

	if (argc != 3) {
		x_perror("usage %s <ip/host> <port/service>", argv[0]);
		return -1;
	}

	TRY
	{
		loop = ev_default_loop(EVBACKEND_EPOLL);
		assert(loop);

		sfd = TcpListen(argv[1], argv[2], NULL, NULL);
		RAISE_SYS_ERROR(sfd);

		FD_Enable(sfd, O_NONBLOCK);

		ev_io_init(&io_w, io_accept, sfd, EV_READ);
		ev_io_start(loop, &io_w);
		ev_loop(loop, 0);
	}
	CATCH
	{
		ev_break(loop, EVBREAK_ALL);
	}
	FINALLY
	{
		ev_loop_destroy(loop);
	}
	END;

	return 0;
}

struct times *_get_time(const char *time)
{
	char            *pos = NULL;
	struct times    *t = NULL;

	t = (struct times *)malloc(sizeof(struct times));

	if (!t) {
		return NULL;
	}

	pos = strchr(time, '.');
	t->sec = atoi(time);
	t->us = atoi(pos + 1);
	return t;
}

void _calculate_diff_time(const char *src, const char *cal, const time_t cln_sec, const int cln_us)
{
	assert(src && cal);

	time_t  src_to_cal_sec = 0;
	int     src_to_cal_us = 0;
	time_t  src_to_cln_sec = 0;
	int     src_to_cln_us = 0;
	time_t  cal_to_cln_sec = 0;
	int     cal_to_cln_us = 0;

	struct times *src_time, *cal_time;
	src_time = _get_time(src);
	cal_time = _get_time(cal);

	if (unlikely(!src_time || !cal_time)) {
		fprintf(stderr, "calculate time error!\n");
		return;
	}

	src_to_cal_sec = (cal_time->sec - src_time->sec) - ((cal_time->us - src_time->us) > 0 ? 0 : 1);
	src_to_cal_us = (1000000 + cal_time->us - src_time->us) % 1000000;

	src_to_cln_sec = (cln_sec - src_time->sec) - ((cln_us - src_time->us) > 0 ? 0 : 1);
	src_to_cln_us = (1000000 + cln_us - src_time->us) % 1000000;

	cal_to_cln_sec = (src_to_cln_sec - src_to_cal_sec) - ((src_to_cln_us - src_to_cal_us) > 0 ? 0 : 1);
	cal_to_cln_us = (1000000 + src_to_cln_us - src_to_cal_us) % 1000000;

	x_printf(I,
		"[%ld.%06d]---"
		"[%ld.%06d]---"
		"[%ld.%06d]",
		src_to_cal_sec, src_to_cal_us,
		src_to_cln_sec, src_to_cln_us,
		cal_to_cln_sec, cal_to_cln_us);

	free(src_time);
	free(cal_time);
}

static void get_diff_time(const char *data, const time_t cln_sec, const int cln_us)
{
	const char      *pos = NULL;
	double          cln_time = 0;
	cJSON           *json;
	cJSON           *src_time, *cal_time;

	assert(data);

	pos = x_strchr(data, strlen(data) + 1, '{');

	if (unlikely(!pos)) {
		return;
	}

	json = cJSON_Parse(pos);

	if (unlikely(!json)) {
		fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
		return;
	}

	src_time = cJSON_GetObjectItem(json, "srchost_time");
	cal_time = cJSON_GetObjectItem(json, CAL_HOST_1);

	if (unlikely((NULL == src_time) || (NULL == cal_time))) {
		fprintf(stderr, "get object error!\n");
		return;
	}

	_calculate_diff_time(src_time->valuestring, cal_time->valuestring, cln_sec, cln_us);
}

static struct client_data *new_client_data(int fd)
{
	assert(fd > -1);
	struct client_data *volatile client_data = NULL;

	TRY
	{
		New(client_data);
		AssertError(client_data, ENOMEM);

		cache_init(&client_data->cache);
		client_data->event.data = client_data;
		client_data->fd = fd;
	}
	CATCH
	{
		if (client_data) {
			cache_free(&client_data->cache);
		}

		Free(client_data);
		RERAISE;
	}
	END;

	return client_data;
}

static void free_client_data(struct client_data **data)
{
	assert(data);

	return_if_fail(*data);
	close((*data)->fd);
	cache_free(&(*data)->cache);
	Free(*data);
}

