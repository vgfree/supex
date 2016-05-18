#pragma once
#include "utils.h"
#include "supex.h"

#define MAX_SNIFF_QUEUE_NUMBER          (MAX_CONNECT + 1)//must > 2
#define MAX_SNIFF_SHARE_QUEUE_NUMBER    (MAX_CONNECT + 1)
#define MAX_SNIFF_DATA_SIZE             15360		// (MAX_REQ_SIZE)

typedef int (*SNIFF_VMS_FCB)(void *user, union virtual_system **VMS, struct sniff_task_node *task);

struct sniff_task_node
{
	struct supex_task_base  base;
	int                     sfd;

	char                    type;
	char                    origin;

	SNIFF_VMS_FCB 		func;
	bool                    last;
	time_t                  stamp;
	pthread_t               thread_id;		/* unique ID of this thread */
	int                     size;
	char                    data[MAX_SNIFF_DATA_SIZE];
	void                    *user;
};
