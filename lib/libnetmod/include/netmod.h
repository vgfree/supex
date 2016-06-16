/***********************************************************************
 * Copyright (C), 2010-2015,
 * ShangHai Language Mirror Automobile Information Technology Co.,Ltd.
 *
 *   File Name: netmod.h
 *      Author: liubaoan@mirrtalk.com
 *     Version: V1.0
 *        Date: 2015/10/19
 *
 * Description:
 *     1. Current netmod.h API support N:1 mode. <N Client and 1 server>.
 *     2. Client's identity must be unique and different.
 *     3. recv_event(),send_event() support multi-threads.
 *
 *      Others:
 *        (null)
 *
 *     History:
 *        Date:           Author:            Modification:
 *
 **********************************************************************/

#ifndef _NETMOD_H_
#define _NETMOD_H_

#include <stdint.h>
#include <pthread.h>
#include "xlist.h"

#define IDENTITY_SIZE 32

/* End-Point type. Server or Client. */
enum node_type
{
	SOCK_SERVER = 1,
	SOCK_CLIENT
};

/* Event type. */
enum event_type
{
	NET_EV_DUMP_REQ = 1,	/* Execute Dump MySQL command request. */
	NET_EV_DUMP_REP,	/* Execute Dump MySQL command response. */
	NET_EV_INCREMENT_REQ,	/* Transfer the Server Queue SQL request. */
	NET_EV_INCREMENT_REP	/* Transfer the Server Queue SQL response. */
};

/* Event execute state. */
enum event_state
{
	NET_EV_NONE = -1,	/* If event is initialize state, then ev_state -1. */
	NET_EV_SUCC,		/* The client execute(Dump MySQL or Execute SQL) succeed. */
	NET_EV_FAIL,		/* The client business is fail, please see the communication
				 *    protocal  with common error message setted in ev_data. */
	NET_EV_FATAL		/* The client business is fatal error, Client will exit.
				 *           with serious error message setted in ev_data. */
};

/* Increment SQL request. */
struct incr_data
{
	uint64_t        task_seq;	/* The sequence of increment SQL request. */
	int             rows;		/* The expect rows of increment data to be send,
					 *    or actual rows of data that client received. */
};

typedef struct _event_t
{
	char    id[IDENTITY_SIZE];	/* Client unique identity. */
	int     ev_type;		/* Event type, EV_XXX_(REQ|REP) */
	int     ev_state;		/* Last event excute state. */
	xlist_t list;			/* List node for recv and send queue. */
	union
	{
		/* Increment SQL request. */
		struct incr_data incr;
	};
	size_t  ev_size;	/* The size of ev_data. if ev_data is null(empty),
				 *   then ev_size must be 0; if ev_data store char* type,
				 *   then ev_size should be strlen(ev_data)+1 for '\0', by yourself. */
	char    ev_data[0];	/* Data to be send to client. */
} event_t;

/* Event Queue Defines. */
typedef struct _queue_ctx_t
{
	xlist_t         head;	// The head node of queue.
	size_t          size;	// Queue's size.
	pthread_cond_t  cond;	// Queue's condition for mutex lock.
	pthread_mutex_t lock;	// Queue's mutex lock.
} queue_ctx_t;

/* Event context define & APIS. */
typedef struct
{
	void            *zmq_ctx;	/* ZeroMQ type's Context. */
	void            *zmq_socket;	/* ZeroMQ type's Socket. */
	void            *zmq_monitor;	/* ZeroMQ event monitor. */
	int             node_type;	/* End point type, Server or Client. */

	queue_ctx_t     *recv_queue;	// receiving-queue of events.
	queue_ctx_t     *send_queue;	// sending-queue of events.

	pthread_t       thrd_kill;	// When call the event_ctx_destroy(), it will execute pthread_kill(thrd, SIGQUIT).
} event_ctx_t;

/* The event_t package head size. */
#define event_head_size()       (sizeof(event_t))

/* The event_t package body size. ev is (event_t *) type. */
#define event_body_size(ptr)    ((size_t)(ptr)->ev_size)

/* The event_t package all size. ev is (event_t *) type. */
#define event_total_size(ptr)   ((size_t)(event_head_size() + event_body_size(ptr)))

/* The event_t package's data. */
#define event_data(ptr)         ((void *)(ptr)->ev_data)

/*************************************************************************
 * FUNCTION:
 *   event_ctx_init().
 * DESCRIPTION:
 *   1. Initialize the network connection and communication;
 *   2. It's ZeroMQ type's socket, at the moment.
 * INPUT:
 *   error:
 *      Input/Output variable, if error occurred, *error will be set.
 *      You can call event_error(error) to get error message.
 *   node_type:
 *      The node type, Client or Server.
 *   uri_addr:
 *      e.g. Client: "tcp://192.168.11.27:8686" Server: "tcp:// *:8686"
 *   identity:
 *      The client identity, It's unique.
 *
 * OUTPUT:
 *   error:  *error will be set with errno.
 *
 * RETURN:
 *   SUCC: event_ctx_t*, a pointer of event_ctx_t type.
 *   FAIL: NULL. You can call event_error() to get error information.
 * OTHERS:
 *   (null)
 ************************************************************************/
event_ctx_t *event_ctx_init(int *error, int node_type, const char *uri_addr, const char *identity);

/*************************************************************************
 * FUNCTION:
 *   event_ctx_destroy().
 * DESCRIPTION:
 *   Destroy the event context resources.
 * INPUT:
 *   ev_ctx:
 *     The event context.
 *
 * RETURN:
 *   SUCC: 0
 *   FAIL: -1, You could call event_error() to get error information.
 * OTHERS:
 *   (null)
 ************************************************************************/
int event_ctx_destroy(event_ctx_t *ev_ctx);

/*************************************************************************
 * FUNCTION:
 *   recv_event().
 * DESCRIPTION:
 *   Server receive the Client's request or Client receive Server's response.
 *
 *   If none of the requested events have occurred on, recv_event() shall
 *   wait timeout milliseconds for an event to occur on.
 *
 *   If timeout is 0, recv_event() shall return immediately.
 *
 *   If timeout is -1, recv_event() shall block indefinitely until a
 *   requested event has occurred on.
 *
 * INPUT:
 *   ev_ctx:
 *     The event context type.
 *   timeout:
 *     Unit(ms).
 *
 * OUTPUT:
 *
 * RETURN:
 *   SUCC: A malloc type of event_t, when used you can call delete_event() to free it.
 *   FAIL: NULL.
 * OTHERS:
 ************************************************************************/
event_t *recv_event(const event_ctx_t *ev_ctx, long timeout);

/*************************************************************************
 * FUNCTION:
 *   event_new_size().
 * DESCRIPTION:
 *   Create a (size)bytes large of event_t to store the ev_data.
 * INPUT:
 *   size: (bytes).
 *
 * RETURN:
 *   SUCC: A malloc type of event_t, when used you can call delete_event() to free it.
 *   FAIL: NULL.
 * OTHERS:
 *   (null)
 ************************************************************************/
event_t *event_new_size(size_t size);

/*************************************************************************
 * FUNCTION:
 *   event_dup().
 * DESCRIPTION:
 *   Duplicate (complete) a new one, from the input entry (event_t *).
 * INPUT:
 *   event: The entry to be duplicated.
 *
 * RETURN:
 *   SUCC: A malloc type of event_t, when used you can call delete_event() to free it.
 *   FAIL: NULL.
 * OTHERS:
 *   (null)
 ************************************************************************/
event_t *event_dup(const event_t *event);

/*************************************************************************
 * FUNCTION:
 *   delete_event().
 * DESCRIPTION:
 *   delete the event_t entry.
 * INPUT:
 *   event:
 *     The event will be delete and destroy.
 *
 * RETURN:
 *   (null)
 * OTHERS:
 *   (null)
 ************************************************************************/
void delete_event(event_t *event);

/*************************************************************************
 * FUNCTION:
 *   send_event().
 * DESCRIPTION:
 *   Send the Client's request or Server's response.
 * INPUT:
 *   ev_ctx
 *     The event context type.
 *   event:
 *     The event will be send.
 *
 * RETURN:
 *   SUCC: 0, The event send succeed.
 *   FAIL: -1, Internal error.
 * OTHERS:
 *   (null)
 ************************************************************************/
int send_event(const event_ctx_t *ev_ctx, const event_t *event);

/*************************************************************************
 * FUNCTION:
 *   event_error().
 * DESCRIPTION:
 *     Return the error message. when netmod.h API's function execute
 *   fail, you can call event_error() to print the error message.
 * INPUT:
 *   error:
 *     The error number, returned by event's API functions.
 * RETURN:
 *   SUCC: The error information.
 *   FAIL: Invalid error number.
 * OTHERS:
 *   (null)
 ************************************************************************/
const char *event_error(int error);

/* print_event & _system, Just for Test */
void print_event(const event_t *event);

const char *_systime();
#endif	/* _NETMOD_H_ */

