#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "pipe.h"

static event_t *__pipe_pull(pipe_t *pipe, int lock, unsigned int millis);

static int __pipe_push(pipe_t *pipe, event_t *event, int end, int lock);

extern void pipe_init(pipe_t *pipe)
{
	list_init(&pipe->head);

	pthread_mutex_init(&pipe->lock, NULL);
	pthread_cond_init(&pipe->cond, NULL);

	pipe->count = 0;
	pipe->wait = 0;
}

extern void pipe_release(pipe_t *pipe)
{
	while (!list_empty(&pipe->head)) {
		xlist_t         *pick = pipe->head.next;
		pipe_entry_t    *entry = container_of(pick, pipe_entry_t, list);

		list_del(pick);
		delete_event(entry->event);
		free(entry);
	}
}

#define PIPE_HEAD_END   0
#define PIPE_TAIL_END   1

extern int pipe_push(pipe_t *pipe, event_t *event)
{
	return __pipe_push(pipe, event, PIPE_TAIL_END, 1);
}

extern int pipe_push_nolock(pipe_t *pipe, event_t *event)
{
	return __pipe_push(pipe, event, PIPE_TAIL_END, 0);
}

extern int pipe_push_back(pipe_t *pipe, event_t *event)
{
	return __pipe_push(pipe, event, PIPE_HEAD_END, 1);
}

extern int pipe_push_back_nolock(pipe_t *pipe, event_t *event)
{
	return __pipe_push(pipe, event, PIPE_HEAD_END, 0);
}

static int __pipe_push(pipe_t *pipe, event_t *event, int end, int lock)
{
	int ret = -1;

	if (pipe && event) {
		pipe_entry_t *entry = (pipe_entry_t *)calloc(1, sizeof(pipe_entry_t));

		if (entry) {
			entry->event = event;
		} else {
			return ret;
		}

		if (lock) {
			pthread_mutex_lock(&pipe->lock);
		}

		if (PIPE_HEAD_END == end) {
			list_add_head(&entry->list, &pipe->head);
		} else if (PIPE_TAIL_END == end) {
			list_add_tail(&entry->list, &pipe->head);
		} else {
			return ret;
		}

		pipe->count++;

		if (lock && pipe->wait) {
			pthread_cond_broadcast(&pipe->cond);
		}

		if (lock) {
			pthread_mutex_unlock(&pipe->lock);
		}

		ret = 0;
	}

	return ret;
}

extern event_t *pipe_pull(pipe_t *pipe, int millis)
{
	return __pipe_pull(pipe, 1, millis);
}

extern event_t *pipe_pull_nolock(pipe_t *pipe)
{
	return __pipe_pull(pipe, 0, 0);
}

static event_t *__pipe_pull(pipe_t *pipe, int lock, unsigned int millis)
{
	event_t *event = NULL;

	struct timeval  now;
	struct timespec timeout;

	if (lock && millis) {
		gettimeofday(&now, NULL);

		if ((now.tv_usec + (suseconds_t)1000 * millis) >= (suseconds_t)1000000) {
			now.tv_sec += 1;
			now.tv_usec += ((suseconds_t)1000000 - (suseconds_t)1000 * millis);
		} else {
			now.tv_usec += (suseconds_t)1000 * millis;
		}

		timeout.tv_sec = now.tv_sec;
		timeout.tv_nsec = now.tv_usec * 1000;
	}

	if (pipe) {
		if (lock) {
			pthread_mutex_lock(&pipe->lock);
		}

		while (0 == pipe->count) {
			if (lock) {
				if (millis) {
					pipe->wait++;
					int res = pthread_cond_timedwait(&pipe->cond, &pipe->lock, &timeout);
					pipe->wait--;

					if ((0 == pipe->count) || (ETIMEDOUT == res)) {
						pthread_mutex_unlock(&pipe->lock);
						return NULL;
					}
				} else {
					pipe->wait++;
					pthread_cond_wait(&pipe->cond, &pipe->lock);
					pipe->wait--;
				}
			} else {
				return NULL;
			}
		}

		xlist_t         *pick = pipe->head.next;
		pipe_entry_t    *entry = container_of(pick, pipe_entry_t, list);

		list_del(pick);	// detach from the chain-list.
		event = entry->event;
		free(entry);

		pipe->count--;

		if (lock) {
			pthread_mutex_unlock(&pipe->lock);
		}
	}

	return event;
}

