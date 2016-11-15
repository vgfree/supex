/**
 * @file   epoll.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 15:44:45 2012
 *
 * @brief  Header file for epoll function declarations.
 *
 *
 */

#ifndef RINOO_EPOLL_H_
#define RINOO_EPOLL_H_

#define RINOO_EPOLL_MAX_EVENTS	128

#include <sys/epoll.h>

struct s_sched;		/* Defined in scheduler.h */
struct s_sched_node;	/* Defined in scheduler.h */
enum e_sched_mode;		/* Defined in scheduler.h */

typedef struct s_epoll {
	int fd;
	int curevent;
	struct epoll_event events[RINOO_EPOLL_MAX_EVENTS];
} t_epoll;

int rinoo_epoll_init(struct s_sched *sched);
void rinoo_epoll_destroy(struct s_sched *sched);
int rinoo_epoll_insert(struct s_sched_node *node, enum e_sched_mode mode);
int rinoo_epoll_addmode(struct s_sched_node *node, enum e_sched_mode mode);
int rinoo_epoll_remove(struct s_sched_node *node);
int rinoo_epoll_poll(struct s_sched *sched, int timeout);

#endif /* !RINOO_RINOO_EPOLL_H_ */
