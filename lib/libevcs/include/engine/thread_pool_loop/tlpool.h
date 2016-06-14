#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	USRSTOP	(SIGSTOP + 32)
#define	USRCONT	(SIGCONT + 32)

enum tlpool_task_type {
	TLPOOL_TASK_SEIZE = 0,
	TLPOOL_TASK_SHARE,
	TLPOOL_TASK_ALONE,
};

typedef struct tlpool tlpool_t;


tlpool_t *tlpool_init(int num_threads, unsigned int max_queue, unsigned int task_size, void *data);

int tlpool_bind(tlpool_t *pool, void (*function)(void *), void *argument, unsigned int index);

int tlpool_boot(tlpool_t *pool);

int tlpool_stop(tlpool_t *pool);

int tlpool_cont(tlpool_t *pool);

int tlpool_exit(tlpool_t *pool);

int tlpool_wait(tlpool_t *pool);

int tlpool_free(tlpool_t *pool);

/*******************************************/
int tlpool_get_thread_index(tlpool_t *pool);

void *tlpool_get_mount_data(tlpool_t *pool);

bool tlpool_push(tlpool_t *pool, void *task, enum tlpool_task_type type, unsigned int index);

bool tlpool_pull(tlpool_t *pool, void *task, enum tlpool_task_type type, unsigned int index);


#ifdef __cplusplus
}
#endif
