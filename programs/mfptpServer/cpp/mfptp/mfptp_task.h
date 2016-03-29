#pragma once
#include "x_utils.h"
#include "def_task.h"

#define MAX_MFPTP_HTTP_NUMBER   MAX_CONNECT
#define MAX_MFPTP_MSMQ_NUMBER   5
#define MAX_MFPTP_PLAN_NUMBER   1
#define MAX_MFPTP_QUEUE_NUMBER  (MAX_MFPTP_HTTP_NUMBER + MAX_MFPTP_MSMQ_NUMBER + MAX_MFPTP_PLAN_NUMBER)

#define MAX_MFPTP_DATA_SIZE     2048			// (MAX_REQ_SIZE)
#define MAX_USR_ID_LEN          16
typedef enum
{
	enMODE_USR = 0,	/* 单用户 */
	enMODE_GRP = 1	/* 群组  */
} dst_mode_t;
struct mfptp_task_node
{
	dst_mode_t      mode;			/* 目的模式，单用户或者群组 */
	char            usrID[MAX_USR_ID_LEN];	/* 用户名称或者群组名称 */
	// TASK_CALLBACK func;
	// int index;
	// char *deal;
	int             data_size;
	void            *data;
	// bool last;
	unsigned int    shift;
	time_t          stamp;
	// pthread_t thread_id;			/* unique ID of this thread */
	// int size;
	// char data[ MAX_MFPTP_DATA_SIZE ];
};

/*---------------------------------------------------------*/

int mfptp_task_rgst(char origin, char type, short workers, short taskers, int mark);

int mfptp_task_over(int id);

void mfptp_task_come(int *store, int id);

int mfptp_task_last(int *store, int id);

