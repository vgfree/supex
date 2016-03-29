#include "http.h"
#include "worker.h"
#include "client.h"

#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define HTTP_API		/* export to external modules */
#define HTTP_INTERNAL_API	/* export to internal modules */

struct http_response
{
	struct event            ev;

	short                   code;
	char                    *body;
	size_t                  body_sz;

	struct http_header      *headers;
	int                     header_count;

	char                    *out;
	size_t                  out_sz;

	int                     http_version, keep_alive, sent;

	struct worker           *w;

	char                    chunked;
};

static int socket_setup(const char *ip, short port);

static void http_response_write(struct http_response *r, int fd);

static void http_response_set_header(struct http_response *r, const char *k, const char *v);

static void set_keepalive_header(struct http_client *c, struct http_response *r);

static struct http_response     *http_response_init(struct worker *w, int code, const char *result, size_t result_sz);

static void server_accept(int fd, short event, void *arg);

static const char *get_status_desc(int code);

static void http_schedule_write(int fd, struct http_response *r);

static void http_can_write(int fd, short event, void *arg);

static void http_response_cleanup(struct http_response *r, int fd, int success);

static char *format_chunk(const char *p, size_t sz, size_t *out_sz);

HTTP_API struct http_server *
new_http_server(const char *host,
	const short port,
	const size_t max_req_size,
	const unsigned int max_worker,
	int (*worker_load_user_data_cb)(struct worker *),
	int (*worker_unload_user_data_cb)(struct worker *),
	struct http_result_data *(*cb)(struct http_req *, void *),
	void *cb_arg)
{
	unsigned int i;

	struct http_server *s = calloc(1, sizeof(*s));

	s->max_worker_thread = max_worker;

	s->http_host = calloc(strlen(host) + 1, 1);
	strcpy(s->http_host, host);
	s->http_port = port;

	s->max_http_request_sz = max_req_size;

	/* set callbacks */
	s->cb = cb;
	s->cb_arg = cb_arg;
	s->worker_load_user_data = worker_load_user_data_cb;
	s->worker_unload_user_data = worker_unload_user_data_cb;

	s->w = calloc(max_worker, sizeof(struct worker));

	for (i = 0; i < max_worker; ++i) {
		s->w[i] = new_worker(s);

		if (s->w[i] == NULL) {
			/* on any worker failing, everything fail */
			free_http_server(s);
			return NULL;
		}
	}

	return s;
}

HTTP_API void
free_http_server(struct http_server *s)
{
	if (s == NULL) {
		return;
	}

	if (s->http_host != NULL) {
		free(s->http_host);
		s->http_host = NULL;
	}

	unsigned int i = 0;

	if (s->w != NULL) {
		for (; i < s->max_worker_thread; ++i) {
			if (s->w[i] != NULL) {
				free_worker(s->w[i]);
			}
		}

		free(s->w);
		s->w = NULL;
	}

	free(s);
}

HTTP_API void
set_http_callback(struct http_server *s,
	struct http_result_data *(*cb)(struct http_req *, void *),
	void *cb_arg)
{
	s->cb = cb;
	s->cb_arg = cb_arg;
}

HTTP_API int
start_http_server(struct http_server *s)
{
	unsigned int    i;
	int             ret;

	s->fd = socket_setup(s->http_host, s->http_port);

	if (s->fd < 0) {
		return -1;
	}

	s->base = event_base_new();

	for (i = 0; i < s->max_worker_thread; ++i) {
		start_worker(s->w[i]);
	}

	event_set(&s->ev, s->fd, EV_READ | EV_PERSIST, server_accept, s);
	event_base_set(s->base, &s->ev);
	ret = event_add(&s->ev, NULL);

	if (ret < 0) {
		return -1;
	}

	event_base_dispatch(s->base);
	return 0;
}

HTTP_API struct http_result_data *
build_res(int http_status, const char *res, size_t res_sz)
{
	struct http_result_data *out = malloc(sizeof(*out));

	memset(out, 0, sizeof(*out));

	out->http_status_code = http_status;

	if (res != NULL) {
		out->result = calloc(1, res_sz + 1);
		memcpy(out->result, res, res_sz + 1);
		out->result_sz = res_sz + 1;
	}

	return out;
}

HTTP_INTERNAL_API size_t
max_request_sz(struct http_server *s)
{
	return s->max_http_request_sz;
}

HTTP_INTERNAL_API void
http_send_response(struct http_client *c, struct http_result_data *res)
{
	struct http_response *resp = http_response_init(NULL,
			res->http_status_code, res->result, res->result_sz);

	resp->http_version = c->http_version;

	set_keepalive_header(c, resp);
	http_response_write(resp, c->fd);
}

static void
server_accept(int fd, short event, void *arg)
{
	struct http_server      *s = arg;
	struct worker           *w;
	struct http_client      *c;
	int                     client_fd;
	struct sockaddr_in      addr;

	socklen_t       addr_sz = sizeof(addr);
	char            on = 1;

	w = s->w[s->next_worker];
	client_fd = accept(fd, (struct sockaddr *)&addr, &addr_sz);

	ioctl(client_fd, (int)FIONBIO, (char *)&on);

	if (client_fd > 0) {
		c = new_http_client(w, client_fd, addr.sin_addr.s_addr);
		worker_add_client(w, c);
		s->next_worker = (s->next_worker + 1) % s->max_worker_thread;
	} else {
		/* FIXME: we should log this */
		return;	/* drop connections silently */
	}
}

static struct http_response *
http_response_init(struct worker *w, int code, const char *result, size_t result_sz)
{
	struct http_response *r = calloc(1, sizeof(*r));

	r->code = code;

	if (result == NULL) {
		r->body = NULL;
		r->body_sz = 0;
	} else {
		r->body = calloc(result_sz + 1, 1);

		memcpy(r->body, result, result_sz);
		r->body_sz = result_sz;
	}

	r->w = w;

	r->keep_alive = 0;	/* default */

	return r;
}

static void
set_keepalive_header(struct http_client *c, struct http_response *r)
{
	r->keep_alive = c->keep_alive;

	if (c->keep_alive) {
		http_response_set_header(r, "Connection", "Keep-Alive");
	} else {
		http_response_set_header(r, "Connection", "Close");
	}
}

static void
http_response_set_header(struct http_response *r, const char *k, const char *v)
{
	int     i, pos = r->header_count;
	size_t  key_sz = strlen(k);
	size_t  val_sz = strlen(v);

	for (i = 0; i < r->header_count; ++i) {
		if (strncmp(r->headers[i].key, k, key_sz) == 0) {
			/* free original key-val header */
			pos = i;
			free(r->headers[i].key);
			free(r->headers[i].val);
			break;
		}
	}

	if (pos == r->header_count) {	/* add a new header */
		r->headers = realloc(r->headers,
				sizeof(*r) * (r->header_count + 1));
		r->header_count++;
	}

	r->headers[pos].key = calloc(key_sz + 1, 1);
	memcpy(r->headers[pos].key, k, key_sz);
	r->headers[pos].key_sz = key_sz;

	r->headers[pos].val = calloc(val_sz + 1, 1);
	memcpy(r->headers[pos].val, v, val_sz);
	r->headers[pos].val_sz = val_sz;

	if (!r->chunked && !strcmp(k, "Transfer-Encoding") && !strcmp(v, "chunked")) {
		r->chunked = 1;
	}
}

static char *
format_chunk(const char *p, size_t sz, size_t *out_sz)
{
	char    *out, tmp[64];
	int     chunk_size;

	/* calculate format size */
	chunk_size = sprintf(tmp, "%x\r\n", (int)sz);

	*out_sz = chunk_size + sz + 2;
	out = malloc(*out_sz);
	memcpy(out, tmp, chunk_size);
	memcpy(out + chunk_size, p, sz);
	memcpy(out + chunk_size + sz, "\r\n", 2);

	return out;
}

static void
http_response_write(struct http_response *r, int fd)
{
	char    *p;
	int     i, ret;

	(void)ret;
	(void)i;
	(void)p;

	const char *status_desc = get_status_desc(r->code);
	r->out_sz = sizeof("HTTP/1.x xxx ") - 1 + strlen(status_desc) + 2;
	r->out = calloc(r->out_sz + 1, 1);

	ret = sprintf(r->out, "HTTP/1.%d %d %s\r\n",
			(r->http_version ? 1 : 0), r->code, status_desc);
	p = r->out;

	if (!r->chunked) {
		char content_length[10];
		snprintf(content_length, 10, "%zd", r->body_sz - 1);
		http_response_set_header(r, "Content-Length", content_length);
	}

	size_t hsz = 0;

	for (i = 0; i < r->header_count; ++i) {
		/* key: value\r\n */
		hsz = r->headers[i].key_sz + 2 + r->headers[i].val_sz + 2;

		/* plus 1 for `\0` in sprintf */
		r->out = realloc(r->out, r->out_sz + hsz + 1);

		p = r->out + r->out_sz;

		sprintf(p, "%s: %s\r\n", r->headers[i].key, r->headers[i].val);
		r->out_sz += hsz;

		if ((strncasecmp("Connection", r->headers[i].key, r->headers[i].key_sz) == 0) &&
			(strncasecmp("Keep-Alive", r->headers[i].val, r->headers[i].val_sz) == 0)) {
			r->keep_alive = 1;
		}
	}

	/* end of headers... */

	/* append body if any */
	if (r->body && r->body_sz) {
		r->out = realloc(r->out, r->out_sz + 2);/* seperator `\r\n` between header and body */
		memcpy(r->out + r->out_sz, "\r\n", 2);
		r->out_sz += 2;

		char    *tmp = (char *)r->body;
		size_t  tmp_len = r->body_sz;

		if (r->chunked) {
			tmp = format_chunk(r->body, r->body_sz, &tmp_len);
		}

		r->out = realloc(r->out, r->out_sz + tmp_len);
		memcpy(r->out + r->out_sz, tmp, tmp_len);

		/* remove the last `\0`, this may cause JSON parse error
		 * on some platform */
		r->out_sz += tmp_len - 1;

		if (r->chunked) {
			free(tmp);
		}
	}

	r->sent = 0;
	http_schedule_write(fd, r);
}

static void
http_schedule_write(int fd, struct http_response *r)
{
	if (r->w) {	/* async */
		event_set(&r->ev, fd, EV_WRITE, http_can_write, r);
		event_base_set(r->w->base, &r->ev);
		event_add(&r->ev, NULL);
	} else {
		/* blocking */
		http_can_write(fd, 0, r);
	}
}

static void
http_can_write(int fd, short event, void *arg)
{
	int                     ret;
	struct http_response    *r = arg;

	/*
	 * To prevent SIGPIPE error to terminate the program,
	 * we use send(2) instead of write(2), because there was
	 * a flag parameter to accept the option MSG_NOSIGNAL(since Linux 2.2)
	 */
	ret = send(fd, r->out + r->sent, r->out_sz - r->sent,
#if defined(__linux__)
			MSG_NOSIGNAL
#elif defined(__APPLE__)
			SO_NOSIGPIPE
#endif
			);

	if (ret > 0) {
		r->sent += ret;
	}

	if ((ret <= 0) || (r->out_sz - r->sent == 0)) {	/* error or done */
		http_response_cleanup(r, fd, (int)r->out_sz == r->sent ? 1 : 0);
	} else {
		http_schedule_write(fd, r);
	}
}

static void
http_response_cleanup(struct http_response *r, int fd, int success)
{
	int i;

	free(r->out);
	free(r->body);

	if (!r->keep_alive || !success) {
		close(fd);
	}

	for (i = 0; i < r->header_count; ++i) {
		free(r->headers[i].key);
		free(r->headers[i].val);
	}

	free(r->headers);
	free(r);
}

static int
socket_setup(const char *ip, short port)
{
	int                     reuse = 1;
	struct sockaddr_in      addr;
	int                     fd, ret;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	addr.sin_addr.s_addr = inet_addr(ip);

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == fd) {
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		return -1;
	}

	ret = fcntl(fd, F_SETFD, O_NONBLOCK);

	if (ret != 0) {
		return -1;
	}

	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));

	if (ret != 0) {
		return -1;
	}

	ret = listen(fd, SOMAXCONN);

	if (0 != ret) {
		return -1;
	}

	return fd;
}

static const char *
get_status_desc(int code)
{
	static const char *status_desc[] = {
		/* 200 */ "OK",
		/* 201 */ "Created",
		/* 202 */ "Accepted",
		/* 204 */ "No Content",
		/* 301 */ "Moved Permanently",
		/* 302 */ "Moved Temporarily",
		/* 304 */ "Not Modified",
		/* 400 */ "Bad Request",
		/* 401 */ "Unauthorized",
		/* 403 */ "Forbidden",
		/* 404 */ "Not Found",
		/* 500 */ "Internal Server Error",
		/* 501 */ "Not Implemented",
		/* 502 */ "Bad Gateway",
		/* 503 */ "Service Unavailable",

		/* default */ "Unknown Error",
	};

	switch (code)
	{
		case 200:
			return status_desc[0];

		case 201:
			return status_desc[1];

		case 202:
			return status_desc[2];

		case 204:
			return status_desc[3];

		case 301:
			return status_desc[4];

		case 302:
			return status_desc[5];

		case 304:
			return status_desc[6];

		case 400:
			return status_desc[7];

		case 401:
			return status_desc[8];

		case 403:
			return status_desc[9];

		case 404:
			return status_desc[10];

		case 500:
			return status_desc[11];

		case 501:
			return status_desc[12];

		case 502:
			return status_desc[13];

		case 503:
			return status_desc[14];

		default:
			return status_desc[sizeof(status_desc) / sizeof(char *) - 1];
	}
}

/* EOF */

