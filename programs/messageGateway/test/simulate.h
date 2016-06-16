#ifndef _SIMULATE_H_
#define _SIMULATE_H_

#include <pthread.h>

void            *g_ctx;
pthread_t       *thread_status;	// 0:push, 1:pull, 2:login, 3:API

void *login_thread(void *usr);

void init_api_server();

void destroy_api_server();

void send_to_api(char *str, int flag);

void *pull_thread(void *usr);

void init_push_server();

void destroy_push_server();
#endif	/* ifndef _SIMULATE_H_ */

