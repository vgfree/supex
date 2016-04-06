#include "mq.h"
#include "thread_condition.h"
#include "spinlock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_QUEUE_SIZE 1024

struct message_queue {
	struct spinlock lock;
	struct thread_condition condition;
	int cap;
	int head;
	int tail;
	int *fd;
	struct router_head *msg_head;
	struct mfptp_message *queue;
};

struct message_queue * 
mq_create() {
	struct message_queue *q = malloc(sizeof(*q));
	q->cap = DEFAULT_QUEUE_SIZE;
	q->head = 0;
	q->tail = 0;
	SPIN_INIT(q)
    CONDITION_INIT(q)
    q->fd = malloc(sizeof(int) * q->cap);
	q->msg_head = malloc(sizeof(struct router_head) * q->cap);
	q->queue = malloc(sizeof(struct mfptp_message) * q->cap);
	return q;
}

static void 
_release(struct message_queue *q) {
	SPIN_DESTROY(q)
    CONDITION_DESTROY(q)
    free(q->fd);
	free(q->msg_head);
	free(q->queue);
	free(q);
}

int
mq_length(struct message_queue *q) {
	int head, tail,cap;

	SPIN_LOCK(q)
	head = q->head;
	tail = q->tail;
	cap = q->cap;
	SPIN_UNLOCK(q)
	
	if (head <= tail) {
		return tail - head;
	}
	return tail + cap - head;
}

int
mq_pop(struct message_queue *q, int *fd,
	   struct router_head *head, struct mfptp_message *message) {
	SPIN_LOCK(q)
    int ret;
	if (q->head != q->tail) {
		*message = q->queue[q->head++];
		*fd = q->fd[q->head];
		*head = q->msg_head[q->head];
		ret = 0;
		int head = q->head;
		int tail = q->tail;
		int cap = q->cap;

		if (head >= cap) {
			q->head = head = 0;
		}
		int length = tail - head;
		if (length < 0) {
			length += cap;
		}
	} else {
		//empty.
		ret = -1;
		CONDITION_WAIT(q);
	}
	
	SPIN_UNLOCK(q)

	return ret;
}

static void
expand_queue(struct message_queue *q) {
    int *new_fd = malloc(sizeof(int) * q->cap * 2);
	struct router_head *new_msg_head = malloc(sizeof(struct router_head) * q->cap * 2);
	struct mfptp_message *new_queue = malloc(sizeof(struct mfptp_message) * q->cap * 2);
	int i;
	for (i=0;i<q->cap;i++) {
		new_fd[i] = q->fd[(q->head + i) % q->cap];
		new_msg_head[i] = q->msg_head[(q->head + i) % q->cap];
		new_queue[i] = q->queue[(q->head + i) % q->cap];
	}
	q->head = 0;
	q->tail = q->cap;
	q->cap *= 2;
	
	free(q->fd);
	free(q->msg_head);
	free(q->queue);
	q->fd = new_fd;
	q->msg_head = new_msg_head;
	q->queue = new_queue;
}

void 
mq_push(struct message_queue *q, int fd,
		struct router_head *head, struct mfptp_message *message) {
	assert(message);
	SPIN_LOCK(q)

	q->fd[q->tail] = fd;
	q->msg_head[q->tail] = *head;
	q->queue[q->tail] = *message;
	if (++ q->tail >= q->cap) {
		q->tail = 0;
	}

	if (q->head == q->tail) {
		expand_queue(q);
	}
	CONDITION_SIGNAL(q);
	SPIN_UNLOCK(q)
}

