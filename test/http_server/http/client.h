#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "http_parser.h"

#include <arpa/inet.h>
#include <event.h>

enum client_error
{
	CLIENT_DISCONNECTED = 0,
	CLIENT_TIMEOUT = -1,
	CLIENT_OOM = -2,
};

struct worker;
struct http_server;

struct http_req
{
	char                    valid;
	char                    method;

	char                    *path;
	size_t                  path_sz;

	struct http_header      *headers;
	int                     nb_header;

	char                    *body;
	size_t                  body_sz;

	size_t                  content_length;
	const char              *last_parse_position;
	struct http_client      *c;
};

struct http_client
{
	int                             fd;
	in_addr_t                       addr;

	struct event                    ev;

	struct worker                   *w;
	struct http_server              *s;

	/* parsing */
	struct http_parser              parser;
	struct http_parser_settings     psr_settings;

	char                            *buffer;
	size_t                          buffer_sz;

	/* flags */
	char                            keep_alive, broken, http_version, failed_alloc, parse_complete;

	struct http_req                 **reqs;
	int                             nb_req;

	struct http_req                 *last_req;

	char                            *uncompleted_bytes;
	size_t                          nb_uncompleted_byte;

	int                             rerun_cnt;
	int                             run_cnt;
};

struct http_client      *new_http_client(struct worker *w, int fd, in_addr_t addr);

int http_client_read(struct http_client *c);

size_t http_client_execute(struct http_client *c);

void http_client_free(struct http_client *c);

void http_client_reset(struct http_client *c);

void incr_run_cnt(struct http_client *c);

void incr_rerun_cnt(struct http_client *c);

void reset_rerun_cnt(struct http_client *c);

int valid_req(struct http_req *r);

void cache_uncompleted_request(struct http_client *c);

void set_client_options(struct http_client *c);
#endif	/* ifndef __CLIENT_H__ */

