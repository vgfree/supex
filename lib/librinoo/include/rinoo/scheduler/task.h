/**
 * @file   task.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Jul  5 17:21:44 2011
 *
 * @brief  Header file for tasks function declarations.
 *
 *
 */

#ifndef RINOO_SCHEDULER_TASK_H_
#define RINOO_SCHEDULER_TASK_H_

#define RINOO_TASK_STACK_SIZE	(16 * 1024)

/* Defined in scheduler.h */
struct s_sched;

typedef struct s_task {
	bool scheduled;
	struct timeval tv;
	struct s_sched *sched;
	t_rbtree_node proc_node;
	t_fcontext context;
	char stack[RINOO_TASK_STACK_SIZE];

#ifdef RINOO_DEBUG
	int valgrind_stackid;
#endif /* !RINOO_DEBUG */
} t_task;

typedef struct s_task_driver {
	t_task main;
	t_task *current;
	t_rbtree proc_tree;
} t_task_driver;

int rinoo_task_driver_init(struct s_sched *sched);
void rinoo_task_driver_destroy(struct s_sched *sched);
int rinoo_task_driver_run(struct s_sched *sched);
int rinoo_task_driver_stop(struct s_sched *sched);
uint32_t rinoo_task_driver_nbpending(struct s_sched *sched);
t_task *rinoo_task_driver_getcurrent(struct s_sched *sched);

t_task *rinoo_task(struct s_sched *sched, t_task *parent, void (*function)(void *arg), void *arg);
void rinoo_task_destroy(t_task *task);
int rinoo_task_start(struct s_sched *sched, void (*function)(void *arg), void *arg);
int rinoo_task_run(struct s_sched *sched, void (*function)(void *arg), void *arg);
int rinoo_task_resume(t_task *task);
int rinoo_task_release(struct s_sched *sched);
int rinoo_task_schedule(t_task *task, struct timeval *tv);
int rinoo_task_unschedule(t_task *task);
int rinoo_task_start(struct s_sched *sched, void (*function)(void *arg), void *arg);
int rinoo_task_wait(struct s_sched *sched, uint32_t ms);
int rinoo_task_pause(struct s_sched *sched);
t_task *rinoo_task_self(void);

#endif /* RINOO_SCHEDULER_TASK_H_ */
