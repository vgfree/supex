/**
 * @file   http.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Apr 15 21:59:15 2012
 *
 * @brief  Header file for http service
 *
 *
 */

#ifndef RINOO_PROTO_HTTP_H_
#define RINOO_PROTO_HTTP_H_

#define RINOO_HTTP_SIGNATURE	"RiNOO/" VERSION

typedef enum e_http_version {
	RINOO_HTTP_VERSION_10 = 0,
	RINOO_HTTP_VERSION_11,
	RINOO_HTTP_VERSION_UNKNOWN
} t_http_version;

typedef struct s_http {
	t_socket *socket;
	t_http_version version;
	t_http_request request;
	t_http_response response;
} t_http;

int rinoo_http_init(t_socket *socket, t_http *http);
void rinoo_http_destroy(t_http *http);
void rinoo_http_reset(t_http *http);

#endif /* !RINOO_PROTO_HTTP_H_ */
