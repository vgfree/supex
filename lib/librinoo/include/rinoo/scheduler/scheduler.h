/**
 * @file   scheduler.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Dec 28 00:14:20 2009
 *
 * @brief  Header file for scheduler function declarations.
 *
 *
 */

#ifndef RINOO_SCHEDULER_SCHEDULER_H_
#define RINOO_SCHEDULER_SCHEDULER_H_

typedef struct s_sched {
	int id;
	bool stop;
	t_list nodes;
	uint32_t nbpending;
	struct timeval clock;
	t_task_driver driver;
	struct s_epoll epoll;
	t_sched_spawns spawns;
} t_sched;

t_sched *rinoo_sched(void);
void rinoo_sched_destroy(t_sched *sched);
int rinoo_sched_spawn(t_sched *sched, int count);
t_sched *rinoo_sched_spawn_get(t_sched *sched, int id);
t_sched *rinoo_sched_self(void);
void rinoo_sched_stop(t_sched *sched);
int rinoo_sched_waitfor(t_sched_node *node,  t_sched_mode mode);
int rinoo_sched_remove(t_sched_node *node);
void rinoo_sched_wakeup(t_sched_node *node, t_sched_mode mode, int error);
int rinoo_sched_poll(t_sched *sched);
void rinoo_sched_loop(t_sched *sched);

#endif /* !RINOO_SCHEDULER_SCHEDULER_H */
