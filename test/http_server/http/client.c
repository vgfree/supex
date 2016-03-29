#include "client.h"
#include "worker.h"
#include "http.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define CHECK_ALLOC(c, ptr)			\
	if (!(ptr)) {				\
		c->failed_alloc = 1; return -1;	\
	}

#define HTTP_INTERNAL_API

#ifndef MAX_TCP_RECV_BUF
  #define MAX_TCP_RECV_BUF 4096
#endif

static int on_url(struct http_parser *p, const char *at, size_t sz);

static int add_body(struct http_client *c, const char *at, size_t sz);

static int on_body(struct http_parser *p, const char *at, size_t sz);

static int on_header_field(struct http_parser *p, const char *at, size_t sz);

static int on_header_value(struct http_parser *p, const char *at, size_t sz);

HTTP_INTERNAL_API struct http_client *
new_http_client(struct worker *w, int fd, in_addr_t addr)
{
	struct http_client *c = calloc(1, sizeof(struct http_client));

	c->fd = fd;
	c->w = w;
	c->addr = addr;
	c->s = w->s;

	http_parser_init(&c->parser, HTTP_REQUEST);
	c->parser.data = c;

	/* callbacks */
	c->psr_settings.on_url = on_url;
	c->psr_settings.on_body = on_body;
	c->psr_settings.on_header_field = on_header_field;
	c->psr_settings.on_header_value = on_header_value;

	return c;
}

HTTP_INTERNAL_API int
http_client_read(struct http_client *c)
{
	char    buffer[MAX_TCP_RECV_BUF];	/* FIXME: */
	int     ret = 0;

	ret = read(c->fd, buffer, sizeof(buffer));

	if (ret <= 0) {
		return ret;
	}

	if (c->buffer_sz > 0) {
		assert(0);
	}

	if (c->nb_uncompleted_byte > 0) {
		/* need to prefix the uncomplete request */
		c->buffer = calloc(c->buffer_sz + c->nb_uncompleted_byte + ret, 1);

		if (!c->buffer) {
			return CLIENT_OOM;
		}

		memcpy(c->buffer, c->uncompleted_bytes, c->nb_uncompleted_byte);
		memcpy(c->buffer + c->nb_uncompleted_byte, buffer, ret);
		c->buffer_sz += (c->nb_uncompleted_byte + ret);

		/* clean uncomplete request bytes */
		free(c->uncompleted_bytes);
		c->nb_uncompleted_byte = 0;
	} else {
		c->buffer = calloc(c->buffer_sz + ret, 1);

		if (!c->buffer) {
			return (int)CLIENT_OOM;
		}

		memcpy(c->buffer + c->buffer_sz, buffer, ret);
		c->buffer_sz += ret;
	}

	return ret;
}

HTTP_INTERNAL_API size_t
http_client_execute(struct http_client *c)
{
	return http_parser_execute(&c->parser,
		       &c->psr_settings, c->buffer, c->buffer_sz);
}

static void
free_request(struct http_req *req)
{
	if (req == NULL) {
		return;
	}

	int idx = 0;

	for (; idx < req->nb_header; ++idx) {
		if (req->headers[idx].key_sz > 0) {
			free(req->headers[idx].key);
			req->headers[idx].key_sz = 0;
		}

		if (req->headers[idx].val_sz > 0) {
			free(req->headers[idx].val);
			req->headers[idx].val_sz = 0;
		}
	}

	if (req->nb_header > 0) {
		free(req->headers);
		req->headers = NULL;
		req->nb_header = 0;
	}

	if (req->path_sz > 0) {
		free(req->path);
		req->path_sz = 0;
	}

	if (req->body_sz) {
		free(req->body);
		req->body = NULL;
		req->body_sz = 0;
	}

	free(req);
}

/* clean all request on the client, and free the request buffer */
static void
clean_request(struct http_client *c)
{
	int idx = 0;

	for (; idx < c->nb_req; ++idx) {
		free_request(c->reqs[idx]);
	}

	if (c->nb_req > 0) {
		free(c->reqs);
		/* set to NULL, or realloc crash */
		c->reqs = NULL;
		c->nb_req = 0;
	}

	if (c->buffer_sz > 0) {
		free(c->buffer);
		c->buffer_sz = 0;
		c->buffer = NULL;
	}
}

static void
free_uncompleted_request(struct http_client *c)
{
	if (c == NULL) {
		return;
	}

	if (c->nb_uncompleted_byte > 0) {
		free(c->uncompleted_bytes);
		c->nb_uncompleted_byte = 0;
	}
}

HTTP_INTERNAL_API void
http_client_free(struct http_client *c)
{
	clean_request(c);
	free_uncompleted_request(c);

	close(c->fd);
	free(c);
}

HTTP_INTERNAL_API int
valid_req(struct http_req *r)
{
	/* FIXME: bad manner, some other method may cause
	 * invalid request, fix that only to disable GET request
	 * to crash http
	 *
	 * We assume all GET request are valid
	 *
	 * Because only POST(currently) can attach body, and only the
	 * `add_body` set the `valid` flag of a request, no other place to
	 * set the flag any more. */
	if ((r->method == HTTP_GET) || (r->valid == 1)) {
		return 1;
	}

	if ((r->method == HTTP_POST) &&
		(r->content_length == 0) && (r->body_sz == 0)) {
		return 1;
	}

	return
		0;
}

HTTP_INTERNAL_API void
http_client_reset(struct http_client *c)
{
	/* append the last uncompleted request
	 * there may be more bytes to make the request complete */
	if (!valid_req(c->last_req)) {
		cache_uncompleted_request(c);
	}

	clean_request(c);

	c->parse_complete = 0;

	if ((c->keep_alive == 0) && (c->nb_uncompleted_byte == 0)) {
		c->broken = 1;
	}

	http_parser_init(&c->parser, HTTP_REQUEST);
}

HTTP_INTERNAL_API void
cache_uncompleted_request(struct http_client *c)
{
	if (c == NULL) {
		return;
	}

	if (c->nb_req == 0) {
		return;
	}

	const char *uncomplete_pos;

	if (valid_req(c->last_req)) {
		return;
	}

	/* get last complete request postion(end of the request)
	 * in current receive buffer */
	if (c->nb_req > 1) {
		uncomplete_pos = c->reqs[c->nb_req - 2]->last_parse_position;
	} else {
		uncomplete_pos = c->reqs[0]->last_parse_position;
	}

	assert(c->nb_uncompleted_byte == 0);

	c->nb_uncompleted_byte = c->buffer_sz - (uncomplete_pos - c->buffer);

	if (c->nb_uncompleted_byte > 0) {
		c->uncompleted_bytes = calloc(c->nb_uncompleted_byte, 1);
		memcpy(c->uncompleted_bytes, uncomplete_pos, c->nb_uncompleted_byte);
		return;
	}

	if (c->nb_uncompleted_byte == 0) {
		/* current request(must be the first one) uncompleted, we should cache
		 * all the receive buffer and receive more bytes to complete it */
		c->uncompleted_bytes = calloc(c->buffer_sz, 1);
		memcpy(c->uncompleted_bytes, c->buffer, c->buffer_sz);
		c->nb_uncompleted_byte = c->buffer_sz;
		return;
	}
}

void incr_run_cnt(struct http_client *c)
{
	c->run_cnt++;
}

void incr_rerun_cnt(struct http_client *c)
{
	c->rerun_cnt++;
}

void reset_rerun_cnt(struct http_client *c)
{
	c->rerun_cnt = 0;
}

static int
on_url(struct http_parser *p, const char *at, size_t sz)
{
	struct http_client *c = p->data;

	struct http_req *new_req = calloc(sizeof(*new_req), 1);

	new_req->path = calloc(sz + 1, 1);
	memcpy(new_req->path, at, sz);
	new_req->path_sz = sz;

	new_req->method = p->method;

	c->reqs = realloc(c->reqs, (c->nb_req + 1) * sizeof(struct http_req *));
	++c->nb_req;
	c->reqs[c->nb_req - 1] = new_req;
	c->last_req = new_req;
	new_req->c = c;

	return 0;
}

HTTP_INTERNAL_API void
set_client_options(struct http_client *c)
{
	if ((c->parser.http_major == 1) && (c->parser.http_minor == 1)) {
		c->keep_alive = 1;	/* HTTP/1.1 */
	}

	c->http_version = c->parser.http_minor;
}

static int
add_body(struct http_client *c, const char *at, size_t sz)
{
	CHECK_ALLOC(c, c->last_req->body = realloc(c->last_req->body,
		c->last_req->body_sz + sz + 1));
	memcpy(c->last_req->body, at, sz);

	c->last_req->body_sz += sz;
	c->last_req->body[c->last_req->body_sz] = 0;

	if (c->last_req->content_length == c->last_req->body_sz) {
		/* FIXME: only here to set the valid of a request */
		c->last_req->valid = 1;
	}

	c->last_req->last_parse_position = at + sz;
	return 0;
}

static int
on_body(struct http_parser *p, const char *at, size_t sz)
{
	struct http_client *c = p->data;

	if (!valid_req(c->last_req)) {
		return add_body(c, at, sz);
	} else {
		return -1;
		/* should not happen on valid HTTP request */
	}
}

static int
on_header_field(struct http_parser *p, const char *at, size_t sz)
{
	struct http_client      *c = p->data;
	struct http_req         *req = c->last_req;

	if (req->valid) {
		/* should not happend on valid HTTP request */
		return -1;
	}

	CHECK_ALLOC(c, req->headers = realloc(req->headers,
		(req->nb_header + 1) * sizeof(struct http_header)))
	++ req->nb_header;

	int n = req->nb_header;
	memset(&req->headers[n - 1], 0, sizeof(struct http_header));

	req->headers[n - 1].key_sz = sz;
	CHECK_ALLOC(c, req->headers[n - 1].key = calloc(sz + 1, 1));
	memcpy(req->headers[n - 1].key, at, sz);
	req->headers[n - 1].key[sz] = 0;

	c->last_req->last_parse_position = at + sz;
	return 0;
}

static int
on_header_value(struct http_parser *p, const char *at, size_t sz)
{
	struct http_client      *c = p->data;
	struct http_req         *req = c->last_req;

	if (req == NULL) {
		/* should not happend on valid HTTP request */
		return -1;
	}

	size_t n = req->nb_header;

	CHECK_ALLOC(c, req->headers[n - 1].val = calloc(1, sz + 1));
	memcpy(req->headers[n - 1].val + req->headers[n - 1].val_sz, at, sz);
	req->headers[n - 1].val_sz += sz;
	req->headers[n - 1].val[req->headers[n - 1].val_sz] = 0;

	if (strncmp("Connection", req->headers[n - 1].key,
		req->headers[n - 1].key_sz) == 0) {
		if ((sz == 10) && (strncmp(at, "Keep-Alive", sz) == 0)) {
			c->keep_alive = 1;
		}
	} else if (strncmp("Content-Length", req->headers[n - 1].key,
		req->headers[n - 1].key_sz) == 0) {
		req->content_length = atol(req->headers[n - 1].val);
	}

	c->last_req->last_parse_position = at + sz;
	return 0;
}

/* EOF */

