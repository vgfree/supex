/**
 * @file   http_response.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Apr 17 17:57:34 2012
 *
 * @brief  Header file for HTTP response
 *
 *
 */

#ifndef RINOO_PROTO_HTTP_RESPONSE_H_
#define RINOO_PROTO_HTTP_RESPONSE_H_

/* Defined in http.h */
struct s_http;

typedef struct s_http_response {
	int code;
	t_buffer msg;
	t_buffer content;
	t_buffer *buffer;
	size_t headers_length;
	size_t content_length;
	t_rbtree headers;
} t_http_response;

int rinoo_http_response_parse(struct s_http *http);
bool rinoo_http_response_get(struct s_http *http);
void rinoo_http_response_setmsg(struct s_http *http, const char *msg);
void rinoo_http_response_setdefaultmsg(struct s_http *http);
void rinoo_http_response_setdefaultheaders(struct s_http *http);
int rinoo_http_response_prepare(struct s_http *http, size_t body_length);
int rinoo_http_response_send(struct s_http *http, t_buffer *body);

#endif /* !RINOO_PROTO_HTTP_RESPONSE_H_ */
