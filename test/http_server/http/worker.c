#include "worker.h"
#include "client.h"
#include "http.h"

#include <stdlib.h>
#include <unistd.h>
#include <event.h>
#include <signal.h>

#define HTTP_INTERNAL_API

#ifndef HTTP_CLIENT_TIMEOUT
  #define HTTP_CLIENT_TIMEOUT 300
#endif

static void *worker_main(void *arg);

static void worker_on_new_client(int pipifd, short event, void *arg);

static void worker_monitor_input(struct http_client *c);

static void worker_can_read(int fd, short event, void *arg);

static int send_response(struct http_client *c, struct http_result_data *res);

HTTP_INTERNAL_API struct worker *
new_worker(struct http_server *s)
{
	struct worker *w = calloc(1, sizeof(struct worker));

	w->s = s;
	w->user_data = NULL;

	/* init communication pipe */
	if (pipe(w->link)) {
		goto fail;
	}

	if (s->worker_load_user_data != NULL) {
		if (s->worker_load_user_data(w)) {
			close(w->link[0]);
			close(w->link[1]);
			goto fail;
		}
	}

	return w;

fail:
	free(w);
	return NULL;
}

HTTP_INTERNAL_API void
start_worker(struct worker *w)
{
	pthread_create(&w->thread, NULL, worker_main, w);
}

HTTP_INTERNAL_API void
worker_add_client(struct worker *w, struct http_client *c)
{
	unsigned long   addr = (unsigned long)c;
	int             ret = write(w->link[1], &addr, sizeof(addr));

	(void)ret;
}

HTTP_INTERNAL_API void
free_worker(struct worker *w)
{
	pthread_kill(w->thread, SIGKILL);
	close(w->link[0]);
	close(w->link[1]);
	struct http_server *s = w->s;

	if (s->worker_unload_user_data && w->user_data) {
		s->worker_unload_user_data(w);
	}

	free(w);
}

static void *
worker_main(void *arg)
{
	struct worker   *w = arg;
	struct event    ev;

	w->base = event_base_new();

	event_set(&ev, w->link[0], EV_READ | EV_PERSIST, worker_on_new_client, w);
	event_base_set(w->base, &ev);
	event_add(&ev, NULL);

	event_base_dispatch(w->base);
	return NULL;
}

HTTP_INTERNAL_API int worker_process_client(struct http_client *c)
{
	struct http_result_data *res;
	struct http_req         *req;
	int                     idx = 0;

	for (; idx < c->nb_req; ++idx) {
		req = c->reqs[idx];

		if (!valid_req(req)) {
			continue;
		}

		res = c->s->cb(req, c->s->cb_arg);
		send_response(c, res);
	}

	return 0;
}

static void
worker_on_new_client(int pipefd, short event, void *arg)
{
	struct http_client      *c;
	unsigned long           addr;

	(void)event;
	(void)arg;

	/* get client from message pipe */
	int ret = read(pipefd, &addr, sizeof(addr));

	if (ret == sizeof(addr)) {
		c = (struct http_client *)addr;
		worker_monitor_input(c);
	}
}

static void
worker_monitor_input(struct http_client *c)
{
	/* set event watcher on client's socket */
	event_set(&c->ev, c->fd, EV_READ | EV_TIMEOUT, worker_can_read, c);
	event_base_set(c->w->base, &c->ev);

	/* add time for each client */
	struct timeval tv;
	tv.tv_sec = HTTP_CLIENT_TIMEOUT;
	tv.tv_usec = 0;

	event_add(&c->ev, &tv);
}

static int
send_response(struct http_client *c, struct http_result_data *res)
{
	if (res == NULL) {
		res = build_res(500, "Server Internal Error", sizeof("Server Internal Error"));
		http_send_response(c, res);
	} else {
		http_send_response(c, res);

		/* TODO: on some @res->http_status_code, we may still
		 * need to set @c->broken flag.
		 */
	}

	free(res->result);
	free(res);

	return 0;
}

static void raw_resp(struct http_client *c, int code)
{
	struct http_result_data *res;

	switch (code)
	{
		case 503:
			res = build_res(503,
					"server internal error",
					sizeof("server internal error"));
			break;

		case 400:
			res = build_res(400,
					"Bad Request",
					sizeof("Bad Request"));
			break;

		case 413:
			res = build_res(413,
					"Request Entity Too Large",
					sizeof("Request Entity Too Large"));
			break;

		default:
			res = build_res(503,
					"server internal error",
					sizeof("server internal error"));
			break;
	}
	send_response(c, res);
}

static void
worker_can_read(int fd, short event, void *arg)
{
	struct http_client      *c = arg;
	int                     ret;
	size_t                  nparsed;

	/* read HTTP request */
	ret = http_client_read(c);

	if (ret <= 0) {
		/* FIXME: should we remove the client due to server fault? */
		if (c->failed_alloc || (ret == CLIENT_OOM)) {
			raw_resp(c, 503);
			return;
		}

		http_client_free(c);
		return;
	}

	nparsed = http_client_execute(c);

	/* we first check parse error */
	if (c->failed_alloc) {
		raw_resp(c, 503);
		c->broken = 1;
	} else if (nparsed != c->buffer_sz) {
		raw_resp(c, 400);
		c->broken = 1;
	} else if (c->buffer_sz > max_request_sz(c->s)) {
		raw_resp(c, 413);
		c->broken = 1;
	} else {
		set_client_options(c);
		worker_process_client(c);
		http_client_reset(c);
	}

	if (c->broken) {
		http_client_free(c);
	} else {
		worker_monitor_input(c);
	}
}

/* EOF */

