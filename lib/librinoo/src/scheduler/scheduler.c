/**
 * @file   scheduler.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Jan  6 16:10:07 2012
 *
 * @brief  Scheduler functions
 *
 *
 */

#include "rinoo/scheduler/module.h"

/**
 * Create a new scheduler.
 *
 *
 * @return Pointer to the new scheduler, or NULL if an error occurs
 */
t_sched *rinoo_sched(void)
{
	t_sched *sched;

	sched = calloc(1, sizeof(*sched));
	if (sched == NULL) {
		return NULL;
	}
	if (rinoo_task_driver_init(sched) != 0) {
		free(sched);
		return NULL;
	}
	if (rinoo_epoll_init(sched) != 0) {
		rinoo_sched_destroy(sched);
		return NULL;
	}
	if (list(&sched->nodes, NULL) != 0) {
		rinoo_sched_destroy(sched);
		return NULL;
	}
	gettimeofday(&sched->clock, NULL);
	return sched;
}

static void rinoo_sched_cancel_task(t_list_node *node)
{
	t_sched_node *sched_node;

	sched_node = container_of(node, t_sched_node, lnode);
	sched_node->error = ECANCELED;
	errno = sched_node->error;
	if (rinoo_task_resume(sched_node->task) != 0 && sched_node->task != NULL) {
		rinoo_task_destroy(sched_node->task);
	}
}

/**
 * Destroy a scheduler
 *
 * @param sched Pointer to the scheduler to destroy
 */
void rinoo_sched_destroy(t_sched *sched)
{
	XASSERTN(sched != NULL);

	rinoo_spawn_destroy(sched);
	rinoo_sched_stop(sched);
	/* Destroying all pending tasks. */
	rinoo_task_driver_stop(sched);
	list_flush(&sched->nodes, rinoo_sched_cancel_task);
	rinoo_task_driver_destroy(sched);
	rinoo_epoll_destroy(sched);
	free(sched);
}

/**
 * Get active scheduler.
 *
 *
 * @return Pointer to the active scheduler or NULL if none.
 */
t_sched *rinoo_sched_self(void)
{
	t_task *task;

	task = rinoo_task_self();
	if (task == NULL) {
		return NULL;
	}
	return task->sched;
}

/**
 * Register a file descriptor in the scheduler and wait for IO.
 *
 * @param node Scheduler node to monitor.
 * @param mode Mode to enable (IN/OUT).
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int rinoo_sched_waitfor(t_sched_node *node, t_sched_mode mode)
{
	int error;

	XASSERT(mode != RINOO_MODE_NONE, -1);

	if (node->error != 0) {
		error = node->error;
		rinoo_sched_remove(node);
		errno = error;
		return -1;
	}
	if ((node->received & mode) == mode) {
		node->received -= mode;
		return 0;
	}
	if ((node->waiting & mode) != mode) {
		if (node->waiting == RINOO_MODE_NONE) {
			if (unlikely(rinoo_epoll_insert(node, mode) != 0)) {
				return -1;
			}
			list_put(&node->sched->nodes, &node->lnode);
		} else {
			if (unlikely(rinoo_epoll_addmode(node, node->waiting | mode) != 0)) {
				return -1;
			}
		}
	}
	node->mode = mode;
	node->waiting |= mode;
	node->task = rinoo_task_driver_getcurrent(node->sched);
	node->sched->nbpending++;
	if (unlikely(node->task == &node->sched->driver.main)) {
		while ((node->received & mode) != mode) {
			rinoo_sched_poll(node->sched);
			if (node->error != 0) {
				error = node->error;
				rinoo_sched_remove(node);
				errno = error;
				node->sched->nbpending--;
				return -1;
			}
		}
		node->sched->nbpending--;
		return 0;
	}
	if (rinoo_task_release(node->sched) != 0 && node->error == 0) {
		node->error = errno;
	}
	node->sched->nbpending--;
	/* Detach task */
	node->task = NULL;
	if (node->error != 0) {
		error = node->error;
		rinoo_sched_remove(node);
		errno = error;
		return -1;
	}
	if ((node->received & mode) != mode) {
		rinoo_sched_remove(node);
		/* Task has been resumed but no event received, this is a timeout */
		errno = ETIMEDOUT;
		return -1;
	}
	node->received -= mode;
	return 0;
}

/**
 * Unregister a file descriptor from the scheduler.
 *
 * @param node Scheduler node to remove.
 *
 * @return 0 on success, otherwise -1.
 */
int rinoo_sched_remove(t_sched_node *node)
{
	if (list_remove(&node->sched->nodes, &node->lnode) != 0) {
		/* Node already removed */
		return -1;
	}
	if (rinoo_epoll_remove(node) != 0) {
		return -1;
	}
	node->task = NULL;
	return 0;
}

/**
 * Wake up a scheduler node task.
 * This function should be called by the file descriptor monitoring layer (epoll).
 *
 * @param node Scheduler node which received IO event.
 * @param mode IO Event.
 * @param error Error flag.
 */
void rinoo_sched_wakeup(t_sched_node *node, t_sched_mode mode, int error)
{
	if (node->error == 0) {
		node->error = error;
	}
	node->received |= mode;
	if (node->task == NULL || node->task == &node->sched->driver.main) {
		return;
	}
	if (node->mode == mode || node->error != 0) {
		rinoo_task_resume(node->task);
	}
}

/**
 * Stops the scheduler. It actually sets the stop flag
 * to end the scheduler loop.
 *
 * @param sched Pointer to the scheduler to stop.
 */
void rinoo_sched_stop(t_sched *sched)
{
	XASSERTN(sched != NULL);

	if (sched->stop == false) {
		sched->stop = true;
		rinoo_spawn_stop(sched);
	}
}

/**
 * Check whether a scheduler has processed all tasks or stop has been requested.
 *
 * @param sched Pointer to the scheduler.
 *
 * @return true if scheduling is over, otherwise false.
 */
static bool rinoo_sched_end(t_sched *sched)
{
	return (sched->stop == true || (sched->nbpending == 0 && rinoo_task_driver_nbpending(sched) == 0));
}

/**
 * Check for any task to be executed and poll hte file descriptor monitoring layer (epoll).
 *
 * @param sched Pointer to the scheduler.
 *
 * @return 0 on success, otherwise -1.
 */
int rinoo_sched_poll(t_sched *sched)
{
	int timeout;

	gettimeofday(&sched->clock, NULL);
	timeout = rinoo_task_driver_run(sched);
	if (!rinoo_sched_end(sched)) {
		return rinoo_epoll_poll(sched, timeout);
	}
	return 0;
}

/**
 * Main scheduler loop.
 * This calls rinoo_sched_poll in a loop until the scheduler gets stopped.
 *
 * @param sched Pointer to the scheduler to use.
 *
 */
void rinoo_sched_loop(t_sched *sched)
{
	sched->stop = false;
	if (rinoo_spawn_start(sched) != 0) {
		goto loop_stop;
	}
	while (!rinoo_sched_end(sched)) {
		rinoo_sched_poll(sched);
	}
loop_stop:
	rinoo_spawn_join(sched);
}
