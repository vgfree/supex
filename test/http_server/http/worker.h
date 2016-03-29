#ifndef __WORKER_H__
#define __WORKER_H__

#include <pthread.h>

struct http_server;
struct http_client;

struct worker
{
	pthread_t               thread;
	struct http_server      *s;
	struct event_base       *base;
	int                     link[2];

	/* each worker may need outter data
	 * these data is access by outter implementation
	 * so there was no handle within HTTP module, none
	 * of HTTP module can access these data;
	 */
	void                    *user_data;
};

#ifdef __cplusplus
extern "C" {
#endif

struct worker   *new_worker(struct http_server *s);

void free_worker(struct worker *w);

void start_worker(struct worker *w);

void worker_add_client(struct worker *w, struct http_client *c);

int worker_process_client(struct http_client *c);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __WORKER_H__ */

