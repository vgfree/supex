/**
 * @file   http_easy.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Apr 30 10:35:46 2013
 *
 * @brief  Easy HTTP interface
 *
 *
 */

#include "rinoo/proto/http/module.h"

/**
 * Calls a HTTP route.
 *
 * @param http Pointer to a HTTP context
 * @param route Pointer to the route to call
 */
static void rinoo_http_easy_route_call(t_http *http, t_http_route *route)
{
	t_buffer body;
	t_buffer *uri;

	http->response.code = route->code;
	switch (route->type) {
	case RINOO_HTTP_ROUTE_STATIC:
		strtobuffer(&body, route->content);
		rinoo_http_response_send(http, &body);
		break;
	case RINOO_HTTP_ROUTE_FUNC:
		if (route->func(http, route) != 0) {
			http->response.code = 500;
			strtobuffer(&body, RINOO_HTTP_ERROR_500);
			rinoo_http_response_send(http, &body);
		}
		break;
	case RINOO_HTTP_ROUTE_FILE:
		if (rinoo_http_send_file(http, route->file) != 0) {
			http->response.code = 404;
			strtobuffer(&body, RINOO_HTTP_ERROR_404);
			rinoo_http_response_send(http, &body);
		}
		break;
	case RINOO_HTTP_ROUTE_DIR:
		uri = buffer_create(NULL);
		buffer_addstr(uri, route->path);
		buffer_addstr(uri, "/");
		buffer_add(uri, buffer_ptr(&http->request.uri), buffer_size(&http->request.uri));
		buffer_addnull(uri);
		if (rinoo_http_send_file(http, buffer_ptr(uri)) != 0) {
			http->response.code = 404;
			strtobuffer(&body, RINOO_HTTP_ERROR_404);
			rinoo_http_response_send(http, &body);
		}
		buffer_destroy(uri);
		break;
	case RINOO_HTTP_ROUTE_REDIRECT:
		rinoo_http_header_set(&http->response.headers, "Location", route->location);
		rinoo_http_response_send(http, NULL);
		break;
	}
}

/**
 * HTTP client processing callback
 *
 * @param context Pointer to a HTTP easy context
 */
static void rinoo_http_easy_client_process(void *context)
{
	int i;
	bool found;
	t_buffer body;
	t_http http;
	t_http_easy_context *econtext = context;

	rinoo_http_init(econtext->socket, &http);
	while (rinoo_http_request_get(&http)) {
		for (i = 0, found = false; i < econtext->nbroutes && found == false; i++) {
			if (econtext->routes[i].uri == NULL ||
			buffer_strcmp(&http.request.uri, econtext->routes[i].uri) == 0 ||
			(econtext->routes[i].type == RINOO_HTTP_ROUTE_DIR && buffer_strncmp(&http.request.uri, econtext->routes[i].uri, strlen(econtext->routes[i].uri)) == 0)) {
				rinoo_http_easy_route_call(&http, &econtext->routes[i]);
				found = true;
			}
		}
		if (found == false) {
			http.response.code = 404;
			strtobuffer(&body, RINOO_HTTP_ERROR_404);
			rinoo_http_response_send(&http, &body);
		}
		rinoo_http_reset(&http);
	}
	rinoo_http_destroy(&http);
	rinoo_socket_destroy(econtext->socket);
	free(econtext);
}

/**
 * HTTP server processing callback
 *
 * @param context Pointer to a HTTP easy context
 */
static void rinoo_http_easy_server_process(void *context)
{
	t_socket *client;
	t_http_easy_context *c_context;
	t_http_easy_context *s_context = context;

	while ((client = rinoo_tcp_accept(s_context->socket, NULL, NULL)) != NULL) {
		c_context = malloc(sizeof(*c_context));
		if (c_context == NULL) {
			rinoo_socket_destroy(client);
			rinoo_socket_destroy(s_context->socket);
			free(s_context);
			return;
		}
		c_context->socket = client;
		c_context->routes = s_context->routes;
		c_context->nbroutes = s_context->nbroutes;
		rinoo_task_start(s_context->socket->node.sched, rinoo_http_easy_client_process, c_context);
	}
	rinoo_socket_destroy(s_context->socket);
	free(s_context);
}

/**
 * Starts a HTTP server which serves the given HTTP easy routes.
 *
 * @param sched Pointer to a scheduler
 * @param ip Ip address to bind
 * @param port Port to bind
 * @param routes Array of HTTP easy route to server
 * @param size Number of routes
 *
 * @return 0 on success, or -1 if an error occurs
 */
int rinoo_http_easy_server(t_sched *sched, t_ip *ip, uint16_t port, t_http_route *routes, int size)
{
	t_socket *server;
	t_http_easy_context *context;

	if (routes == NULL) {
		return -1;
	}
	server = rinoo_tcp_server(sched, ip, port);
	if (server == NULL) {
		return -1;
	}
	context = malloc(sizeof(*context));
	if (context == NULL) {
		return -1;
	}
	context->socket = server;
	context->routes = routes;
	context->nbroutes = size;
	if (rinoo_task_start(sched, rinoo_http_easy_server_process, context) != 0) {
		free(context);
		return -1;
	}
	return 0;
}
