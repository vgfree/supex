#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "router.h"

#include <stdlib.h>
#include <stdint.h>

struct message_queue;

struct message_queue * mq_create();

// 0 for success
int mq_pop(struct message_queue *q, int *fd,
	   struct router_head *head, struct mfptp_message *message);
void mq_push(struct message_queue *q, int fd,
	     struct router_head *head, struct mfptp_message *message);

// return the length of message queue, for debug
int mq_length(struct message_queue *q);

#endif
