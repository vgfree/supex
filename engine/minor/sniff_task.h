#pragma once
#include "scco.h"
#include "utils.h"
#include "def_task.h"
#include "supex.h"

// #define MAX_SNIFF_QUEUE_NUMBER			(MAX_CONNECT * 2)//must > 2
#define MAX_SNIFF_QUEUE_NUMBER          (MAX_CONNECT + 1)
// #define MAX_SNIFF_SHARE_QUEUE_NUMBER		(MAX_SNIFF_DATA_SIZE * 16)
#define MAX_SNIFF_SHARE_QUEUE_NUMBER    (MAX_CONNECT + 1)
// #define MAX_SNIFF_DATA_SIZE			6144//(MAX_REQ_SIZE)
#define MAX_SNIFF_DATA_SIZE             15360		// (MAX_REQ_SIZE)

struct sniff_task_node
{
	struct supex_task_base  base;
	int                     sfd;

	char                    type;
	char                    origin;
	SUPEX_TASK_CALLBACK     func;

	bool                    last;
	time_t                  stamp;
	pthread_t               thread_id;		/* unique ID of this thread */
	int                     size;
	char                    data[MAX_SNIFF_DATA_SIZE];
	void                    *user;
};

