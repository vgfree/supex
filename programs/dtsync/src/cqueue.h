/*
 * cqueue.h
 *
 *  Created on: 2015/10/17
 *      Author: liubaoan@mirrtalk.com
 */

#ifndef SRC_CQUEUE_H_
#define SRC_CQUEUE_H_

#include <pthread.h>

/* Structure of Queue Element */
typedef struct _MCQElement
{
	struct _MCQElement      *prev;
	unsigned char           iQEleType;
	void                    *pValue;
	struct _MCQElement      *next;
} MCQElement;

/* Structure of Queue */
typedef struct _MCQueue
{
	int             QNumber;
	MCQElement      *QHead;
	MCQElement      *QTail;
	pthread_mutex_t QLock;
} MCQueue;

/* Initizalize the MCQueue */
extern MCQueue *initMCQueue(int *error);

/* Add MCQElement to MCQueue at queue tail */
extern int enMCQueue(MCQueue *q, MCQElement *el);

/* Detach MCQElement from MCQueue head */
extern int deMCQueue(MCQueue *q, MCQElement **el);

/* Length of MCQueue */
extern int mcQueueLength(MCQueue *q);

/* Destroy the MCQueue */
extern int destroyMCQueue(MCQueue *q);
#endif	/* SRC_CQUEUE_H_ */

