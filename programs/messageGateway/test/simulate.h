#ifndef _SIMULATE_H_
#define _SIMULATE_H_

#include <pthread.h>

void *g_ctx;
pthread_t *thread_status; // 0:push, 1:pull, 2:login, 3:API

void *login_thread(void *usr);
void *api_thread(void *usr);
void *push_thread(void *usr);
void *pull_thread(void *usr);
#endif
