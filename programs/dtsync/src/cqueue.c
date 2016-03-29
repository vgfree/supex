/*
 * cqueue.c
 *
 *  Created on: 2015/10/17
 *      Author: liubaoan@mirrtalk.com
 */

#include "cqueue.h"
#include <errno.h>
#include <stdlib.h>

/* Initizalize the MCQueue */
MCQueue *initMCQueue(int *error)
{
	if (error == NULL) {
		return NULL;
	}

	MCQueue *iLink = NULL;
	iLink = (MCQueue *)malloc(sizeof(MCQueue));

	if (iLink == NULL) {
		*error = errno;
		return NULL;
	}

	if (pthread_mutex_init(&iLink->QLock, NULL)) {
		free(iLink);
		return NULL;
	}

	iLink->QNumber = 0;
	iLink->QHead = NULL;
	iLink->QTail = NULL;
	return iLink;
}

/* Add MCQElement to MCQueue at queue tail */
int enMCQueue(MCQueue *iLink, MCQElement *iNode)
{
	if ((iLink == NULL) || (iNode == NULL)) {
		return -1;
	}

	pthread_mutex_lock(&iLink->QLock);

	if (iLink->QTail == NULL) {
		iLink->QHead = iLink->QTail = iNode;
		iLink->QNumber = 1;
	} else {
		iLink->QTail->next = iNode;
		iNode->prev = iLink->QTail;
		iLink->QTail = iNode;
		iLink->QNumber++;
	}

	pthread_mutex_unlock(&iLink->QLock);
	return 0;
}

/* Detach MCQElement from MCQueue head */
int deMCQueue(MCQueue *iLink, MCQElement **el)
{
	if ((iLink == NULL) || (el == NULL)) {
		return -1;
	}

	pthread_mutex_lock(&iLink->QLock);

	if (iLink->QHead == NULL) {
		*el = NULL;
		pthread_mutex_unlock(&iLink->QLock);
		return 1;
	}

	MCQElement *p = NULL, *q = NULL;

	if (iLink->QHead->next == NULL) {
		p = iLink->QHead;
		iLink->QHead = iLink->QTail = NULL;
		iLink->QNumber = 0;
		p->next = p->prev = NULL;
		*el = p;
	} else {
		p = iLink->QHead->next;
		q = iLink->QHead;
		q->next = q->prev = NULL;
		*el = q;
		p->prev = NULL;
		iLink->QHead = p;
		iLink->QNumber--;
	}

	pthread_mutex_unlock(&iLink->QLock);
	return 0;
}

/* Length of MCQueue */
int mcQueueLength(MCQueue *iLink)
{
	if (iLink == NULL) {
		return -1;
	}

	return iLink->QNumber;
}

/* Destroy the MCQueue */
int destroyMCQueue(MCQueue *iLink)
{
	if (iLink == NULL) {
		return -1;
	}

	pthread_mutex_destroy(&iLink->QLock);
	free(iLink);

	return 0;
}

