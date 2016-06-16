#include "event_pipe.h"
#include <stdlib.h>
#include <assert.h>

evpipe_t *evpipe_init()
{
	evpipe_t *p_evp = (evpipe_t *)malloc(sizeof(evpipe_t));

	assert(p_evp != NULL);

	//	pthread_cond_t *p_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	//	assert(p_cond!= NULL);

	list_init(&p_evp->head);
	p_evp->ele_counts = 0;
	//	p_evp->okay = 0;
	//	int res = pthread_cond_init(p_cond, NULL);
	//	assert(res == 0);

	return p_evp;
}

void evpipe_release(evpipe_t *pipe)
{
	assert(pipe != NULL);

	xlist_t         *iter = NULL;
	evpipe_ele_t    *ele = NULL;

	while (!list_empty(&pipe->head)) {
		iter = pipe->head.next;
		ele = container_of(iter, evpipe_ele_t, node);

		list_del(iter);
		delete_event(ele->ev);
		free(ele);
	}

	//	pthread_cond_destroy(pipe->cond);
	free(pipe);
}

static int _pipe_push(evpipe_t *pipe, event_t *ev, int position)
{
	assert(pipe != NULL && ev != NULL);

	int res = -1;

	if (pipe && ev) {
		evpipe_ele_t *ele = (evpipe_ele_t *)calloc(1, sizeof(evpipe_ele_t));

		if (!ele) {
			return res;
		}

		ele->ev = ev;

		(position == 1) ?
		list_add_head(&ele->node, &pipe->head) :
		list_add_tail(&ele->node, &pipe->head)
		;
		++(pipe->ele_counts);

		//		if (pipe->okay)
		//			pthread_cond_broadcast(pipe->cond);

		return 0;
	}

	return res;
}

int evpipe_push_head(evpipe_t *pipe, event_t *ev)
{
	return _pipe_push(pipe, ev, 1);
}

int evpipe_push_tail(evpipe_t *pipe, event_t *ev)
{
	return _pipe_push(pipe, ev, -1);
}

int evpipe_length(evpipe_t *pipe)
{
	assert(pipe != NULL);

	return pipe->ele_counts;
}

event_t *evpipe_pull(evpipe_t *pipe)
{
	event_t *ev = NULL;

	if (pipe && (pipe->ele_counts > 0)) {
		xlist_t         *iter = pipe->head.next;
		evpipe_ele_t    *ele = container_of(iter, evpipe_ele_t, node);

		list_del(iter);
		ev = ele->ev;
		free(ele);

		--(pipe->ele_counts);
	}

	return ev;
}

