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
 *     3. recv_evt(),send_evt() support multi-threads.
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
#include "base/xlist.h"
#include "base/qlist.h"

#define IDENTITY_SIZE 32

/* End-Point type. Server or Client. */
enum node_type
{
	SOCK_SERVER = 1,
	SOCK_CLIENT
};

/* Event type. */
enum evt_type
{
	NET_EV_DUMP_REQ = 1,	/* Execute Dump MySQL command request. */
	NET_EV_DUMP_REP,	/* Execute Dump MySQL command response. */
	NET_EV_INCREMENT_REQ,	/* Transfer the Server Queue SQL request. */
	NET_EV_INCREMENT_REP	/* Transfer the Server Queue SQL response. */
};

/* Event execute state. */
enum evt_state
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

typedef struct _evt
{
	char    id[IDENTITY_SIZE];	/* Client unique identity. */
	enum evt_type     ev_type;		/* Event type, EV_XXX_(REQ|REP) */
	enum evt_state     ev_state;		/* Last event excute state. */
	union
	{
		/* Increment SQL request. */
		struct incr_data incr;
	};
	size_t  ev_size;	/* The size of ev_data. if ev_data is null(empty),
				 *   then ev_size must be 0; if ev_data store char* type,
				 *   then ev_size should be strlen(ev_data)+1 for '\0', by yourself. */
	char    ev_data[0];	/* Data to be send to client. */
} evt_t;

/* Event context define & APIS. */
typedef struct
{
	void                    *zmq_ctx;	/* ZeroMQ type's Context. */
	void                    *zmq_socket;	/* ZeroMQ type's Socket. */
	void                    *zmq_monitor;	/* ZeroMQ event monitor. */
	enum node_type                     type;	/* End point type, Server or Client. */

	QLIST       qrecv;		// receiving-queue of events.
	QLIST       qsend;		// sending-queue of events.

	pthread_t               thrd_kill;	// When call the evt_ctx_destroy(), it will execute pthread_kill(thrd, SIGQUIT).
} evt_ctx_t;

/* The evt_t package head size. */
#define evt_head_size()       (sizeof(evt_t))

/* The evt_t package body size. ev is (evt_t *) type. */
#define evt_body_size(ptr)    ((size_t)(ptr)->ev_size)

/* The evt_t package all size. ev is (evt_t *) type. */
#define evt_total_size(ptr)   ((size_t)(evt_head_size() + evt_body_size(ptr)))

/* The evt_t package's data. */
#define evt_body_data(ptr)         ((void *)(ptr)->ev_data)

/*************************************************************************
 * FUNCTION:
 *   evt_ctx_init().
 * DESCRIPTION:
 *   1. Initialize the network connection and communication;
 *   2. It's ZeroMQ type's socket, at the moment.
 * INPUT:
 *   error:
 *      Input/Output variable, if error occurred, *error will be set.
 *      You can call evt_error(error) to get error message.
 *   type:
 *      The node type, Client or Server.
 *   uri_addr:
 *      e.g. Client: "tcp://192.168.11.27:8686" Server: "tcp:// *:8686"
 *   identity:
 *      The client identity, It's unique.
 *
 * OUTPUT:
 *   error will be set with errno.
 *
 * RETURN:
 *   SUCC: evt_ctx_t*, a pointer of evt_ctx_t type.
 *   FAIL: NULL. You can call evt_error() to get error information.
 * OTHERS:
 *   (null)
 ************************************************************************/
evt_ctx_t *evt_ctx_init(enum node_type type, const char *uri_addr, const char *identity);

/*************************************************************************
 * FUNCTION:
 *   evt_ctx_destroy().
 * DESCRIPTION:
 *   Destroy the event context resources.
 * INPUT:
 *   evt_ctx:
 *     The event context.
 *
 * RETURN:
 *   SUCC: 0
 *   FAIL: -1, You could call evt_error() to get error information.
 * OTHERS:
 *   (null)
 ************************************************************************/
int evt_ctx_destroy(evt_ctx_t *evt_ctx);

/*************************************************************************
 * FUNCTION:
 *   evt_new_by_size().
 * DESCRIPTION:
 *   Create a (size)bytes large of evt_t to store the ev_data.
 * INPUT:
 *   size: (bytes).
 *
 * RETURN:
 *   SUCC: A malloc type of evt_t, when used you can call free_evt() to free it.
 *   FAIL: NULL.
 * OTHERS:
 *   (null)
 ************************************************************************/
evt_t *evt_new_by_size(size_t size);

/*************************************************************************
 * FUNCTION:
 *   copy_evt().
 * DESCRIPTION:
 *   Duplicate (complete) a new one, from the input entry (evt_t *).
 * INPUT:
 *   event: The entry to be duplicated.
 *
 * RETURN:
 *   SUCC: A malloc type of evt_t, when used you can call free_evt() to free it.
 *   FAIL: NULL.
 * OTHERS:
 *   (null)
 ************************************************************************/
evt_t *copy_evt(const evt_t *event);

/*************************************************************************
 * FUNCTION:
 *   free_evt().
 * DESCRIPTION:
 *   delete the evt_t entry.
 * INPUT:
 *   event:
 *     The event will be delete and destroy.
 *
 * RETURN:
 *   (null)
 * OTHERS:
 *   (null)
 ************************************************************************/
void free_evt(evt_t *event);

/*************************************************************************
 * FUNCTION:
 *   send_evt().
 * DESCRIPTION:
 *   Send the Client's request or Server's response.
 * INPUT:
 *   evt_ctx
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
int send_evt(evt_ctx_t *evt_ctx, evt_t *event);

/*************************************************************************
 * FUNCTION:
 *   recv_evt().
 * DESCRIPTION:
 *   Server receive the Client's request or Client receive Server's response.
 *
 *   If none of the requested events have occurred on, recv_evt() shall
 *   wait timeout milliseconds for an event to occur on.
 *
 *   If timeout is 0, recv_evt() shall return immediately.
 *
 *   If timeout is -1, recv_evt() shall block indefinitely until a
 *   requested event has occurred on.
 *
 * INPUT:
 *   evt_ctx:
 *     The event context type.
 *   timeout:
 *     Unit(ms).
 *
 * OUTPUT:
 *
 * RETURN:
 *   SUCC: A malloc type of evt_t, when used you can call free_evt() to free it.
 *   FAIL: NULL.
 * OTHERS:
 ************************************************************************/
evt_t *recv_evt(evt_ctx_t *evt_ctx);

/*************************************************************************
 * FUNCTION:
 *   evt_error().
 * DESCRIPTION:
 *     Return the error message. when netmod.h API's function execute
 *   fail, you can call evt_error() to print the error message.
 * INPUT:
 *   error:
 *     The error number, returned by event's API functions.
 * RETURN:
 *   SUCC: The error information.
 *   FAIL: Invalid error number.
 * OTHERS:
 *   (null)
 ************************************************************************/
const char *evt_error(int error);

/* print_evt & _system, Just for Test */
void print_evt(const evt_t *event);

void *work_evt(evt_ctx_t *evt_ctx);

#endif	/* _NETMOD_H_ */

