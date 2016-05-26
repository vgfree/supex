#pragma once
#include "utils.h"

__BEGIN_DECLS

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
	X_NEW_LOCK      lock;
};

QITEM *qitem_init(QITEM *item);

void qitem_free(QITEM *item);

void qlist_init(QLIST *list);

QITEM *qlist_pull(QLIST *list);

void qlist_push(QLIST *list, QITEM *item);

int qlist_view(QLIST *list);

/* ----------------                 */

/* ----------------                 */
__END_DECLS

