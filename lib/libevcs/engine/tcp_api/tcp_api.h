#pragma once

#include "scco.h"
#include "evcoro_scheduler.h"

#define TCP_ERR_CONNECT                 -1
#define TCP_ERR_SEND                    -2
#define TCP_ERR_SOCKOPT                 -3
#define TCP_ERR_MEMORY                  -4
#define TCP_ERR_TIMEOUT                 -5

#define DEFAULT_RECV_SIZE               20480

#define MAX_IDLE_SWITCH_TIME_OUT        3500

typedef int (*ASK_IDLE_CB)(void *user);

int x_connect(const char *host, int port, size_t cnt_timeout);

/**
 * @param time > 0 超时毫秒数
 */
int sync_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t cnt_timeout, size_t rcv_timeout);

/**
 * @param time > 0 超时微秒数
 */
int async_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t cnt_timeout, size_t rcv_timeout, ASK_IDLE_CB func_idle_cb, void *user);

int http_get_body_length(char *buf, size_t size);

int http_get_head_length(char *buf, size_t size);

