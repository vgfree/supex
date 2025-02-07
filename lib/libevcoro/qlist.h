#pragma once
#include <stdint.h>
#include <pthread.h>


/* An item in the connection queue. */
typedef struct queue_item QITEM;
struct queue_item
{
	char                    come;
	void                    *data;
	struct queue_item       *next;
};

/* A connection queue. */
typedef struct queue_list QLIST;
struct queue_list
{
	int             nubs;
	QITEM           *head;
	QITEM           *tail;
	pthread_mutex_t lock;
};

QITEM *qitem_init(QITEM *item);

void qitem_free(QITEM *item);

void qlist_init(QLIST *list);

QITEM *qlist_pull(QLIST *list);

void qlist_push(QLIST *list, QITEM *item);

void qlist_back(QLIST *list, QITEM *item);

void qlist_free(QLIST *list);

int qlist_view(QLIST *list);

int qlist_lock(QLIST *list);

int qlist_unlock(QLIST *list);

QITEM *qlist_nolock_pull(QLIST *list);

void qlist_nolock_push(QLIST *list, QITEM *item);
