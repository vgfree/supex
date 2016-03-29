#ifndef __HTTP_H__
#define __HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <event.h>

struct worker;
struct http_client;
struct http_req;

struct http_server
{
	int                     fd;
	struct event            ev;
	struct event_base       *base;

	size_t                  max_http_request_sz;

	char                    *http_host;
	short                   http_port;

	/* worker thread */
	struct worker           **w;
	int                     next_worker;
	unsigned int            max_worker_thread;

	int                     (*worker_load_user_data)(struct worker *);
	int                     (*worker_unload_user_data)(struct worker *);

	struct http_result_data *(*cb)(struct http_req *, void *);
	void                    *cb_arg;
};

struct http_header
{
	char    *key, *val;
	size_t  key_sz, val_sz;
};

struct http_result_data
{
	int     http_status_code;
	char    *result;
	size_t  result_sz;
};

struct http_server;
struct http_client;
struct http_server;

struct http_server *
new_http_server(const char *host,
	const short port,
	const size_t max_req_sz,
	const unsigned int max_worker,
	int (*worker_load_user_data_cb)(struct worker *),
	int (*worker_unload_user_data_cb)(struct worker *),
	struct http_result_data *(*cb)(struct http_req *, void *),
	void *cb_arg);

void
free_http_server(struct http_server *s);

int
start_http_server(struct http_server *s);

size_t
max_request_sz(struct http_server *s);

void
http_send_response(struct http_client *c, struct http_result_data *res);

struct http_result_data *
build_res(int http_status, const char *internal_errmsg, size_t msg_sz);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __HTTP_H__ */

