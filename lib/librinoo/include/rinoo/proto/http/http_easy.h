/**
 * @file   http_easy.h
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Apr 30 10:14:57 2013
 *
 * @brief  Easy HTTP interface
 *
 *
 */

#ifndef RINOO_PROTO_HTTP_EASY_H_
#define RINOO_PROTO_HTTP_EASY_H_

#define RINOO_HTTP_ERROR_500	"<div style=\"display: inline-block; border-radius: 4px; border: 1px solid red; width: 16px; height: 16px; color: red; font-size: 14px; text-align: center;\">&#10060;</div> <span style=\"font-family: Arial;\">500 - Internal server error</span>"
#define RINOO_HTTP_ERROR_404	"<div style=\"display: inline-block; border-radius: 4px; border: 1px solid orange; width: 16px; height: 16px; color: orange; font-size: 14px; text-align: center;\">?</div> <span style=\"font-family: Arial;\">404 - Not found</span>"

typedef enum e_http_route_type {
	RINOO_HTTP_ROUTE_STATIC = 0,
	RINOO_HTTP_ROUTE_FUNC,
	RINOO_HTTP_ROUTE_FILE,
	RINOO_HTTP_ROUTE_DIR,
	RINOO_HTTP_ROUTE_REDIRECT,
} t_http_route_type;

typedef struct s_http_route {
	const char *uri;
	int code;
	t_http_route_type type;
	union {
		const char *file;
		const char *path;
		const char *content;
		const char *location;
		int (*func)(t_http *http, struct s_http_route *route);
	};
} t_http_route;

typedef struct s_http_easy_context {
	int nbroutes;
	t_socket *socket;
	t_http_route *routes;
} t_http_easy_context;

int rinoo_http_easy_server(t_sched *sched, t_ip *ip, uint16_t port, t_http_route *routes, int size);

#endif /* !RINOO_PROTO_HTTP_EASY_H_ */
