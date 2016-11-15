/**
 * @file   http_request.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Apr 17 17:55:30 2012
 *
 * @brief  Header file for HTTP request
 *
 *
 */

#ifndef RINOO_PROTO_HTTP_REQUEST_H_
#define RINOO_PROTO_HTTP_REQUEST_H_

/* Defined in http.h */
struct s_http;

typedef enum e_http_method {
	RINOO_HTTP_METHOD_UNKNOWN = 0,
	RINOO_HTTP_METHOD_OPTIONS = 1,
	RINOO_HTTP_METHOD_GET = 2,
	RINOO_HTTP_METHOD_HEAD = 4,
	RINOO_HTTP_METHOD_POST = 8,
	RINOO_HTTP_METHOD_PUT = 16,
	RINOO_HTTP_METHOD_DELETE = 32,
	RINOO_HTTP_METHOD_TRACE = 64,
	RINOO_HTTP_METHOD_CONNECT = 128,
} t_http_method;

typedef struct s_http_request {
	t_buffer uri;
	t_buffer content;
	t_buffer *buffer;
	size_t headers_length;
	size_t content_length;
	t_rbtree headers;
	t_http_method method;
} t_http_request;

int rinoo_http_request_parse(struct s_http *http);
bool rinoo_http_request_get(struct s_http *http);
void rinoo_http_request_setdefaultheaders(struct s_http *http);
int rinoo_http_request_send(struct s_http *http, t_http_method method, const char *uri, t_buffer *body);

#endif /* !RINOO_PROTO_HTTP_REQUEST_H_ */
