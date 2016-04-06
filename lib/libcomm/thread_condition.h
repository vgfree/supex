
#ifndef CONDITION_H
#define CONDITION_H

#define CONDITION_INIT(q) condition_init(&(q)->condition);
#define CONDITION_SIGNAL(q) condition_signal(&(q)->condition);
#define CONDITION_WAIT(q) condition_wait(&(q)->condition, &(q)->lock);
#define CONDITION_DESTROY(q) condition_destroy(&(q)->condition);


#include "spinlock.h"

#include <stdint.h>

struct thread_condition
{
  pthread_cond_t condition;
};

void condition_init(struct thread_condition *tc)
{
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  pthread_cond_init(&tc->condition, &attr);
  pthread_condattr_destroy(&attr);
}

void condition_destroy(struct thread_condition *tc)
{
  pthread_cond_destroy(&tc->condition);
}

void condition_signal(struct thread_condition *tc)
{
  pthread_cond_signal(&tc->condition);
}

void condition_broadcast(struct thread_condition *tc)
{
  pthread_cond_broadcast(&tc->condition);
}

void condition_wait(struct thread_condition *tc,
		    struct spinlock *lock)
{
  pthread_cond_wait(&tc->condition, &lock->lock);
}

#endif
