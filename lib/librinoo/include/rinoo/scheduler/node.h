/**
 * @file   node.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Dec 28 00:14:20 2009
 *
 * @brief  Header file for scheduler node structures.
 *
 *
 */

#ifndef RINOO_SCHEDULER_NODE_H_
#define RINOO_SCHEDULER_NODE_H_

/* Declared in scheduler.h */
struct s_sched;

typedef enum e_sched_mode {
	RINOO_MODE_NONE = 0,
	RINOO_MODE_IN = 1,
	RINOO_MODE_OUT = 2,
} t_sched_mode;

typedef struct s_sched_node {
	int fd;
	int error;
	t_task *task;
	t_list_node lnode;
	t_sched_mode mode;
	t_sched_mode waiting;
	t_sched_mode received;
	struct s_sched *sched;
} t_sched_node;

#endif /* !RINOO_SCHEDULER_NODE_H_ */
