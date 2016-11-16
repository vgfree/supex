/**
 * @file   task.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Oct 13 22:15:13 2011
 *
 * @brief  Scheduler task functions
 *
 *
 */

#include "rinoo/scheduler/module.h"

#ifdef RINOO_DEBUG
# include <valgrind/valgrind.h>
#endif

static __thread t_task *current_task = NULL;

static int rinoo_task_cmp(t_rbtree_node *node1, t_rbtree_node *node2)
{
	t_task *task1 = container_of(node1, t_task, proc_node);
	t_task *task2 = container_of(node2, t_task, proc_node);

	if (task1 == task2) {
		return 0;
	}
	if (timercmp(&task1->tv, &task2->tv, <)) {
		return -1;
	}
	return 1;
}

/**
 * Task driver initialization.
 * It sets the task driver in a scheduler.
 *
 * @param sched Pointer to the scheduler to set
 *
 * @return 0 on success, -1 if an error occurs
 */
int rinoo_task_driver_init(t_sched *sched)
{
	XASSERT(sched != NULL, -1);

	if (rbtree(&sched->driver.proc_tree, rinoo_task_cmp, NULL) != 0) {
		return -1;
	}
	sched->driver.main.sched = sched;
	sched->driver.current = &sched->driver.main;
	current_task = &sched->driver.main;
	return 0;
}

/**
 * Destroy internal task driver from a scheduler.
 *
 * @param sched Pointer to the scheduler to use
 */
void rinoo_task_driver_destroy(t_sched *sched)
{
	XASSERTN(sched != NULL);

	rbtree_flush(&sched->driver.proc_tree);
}

/**
 * Runs pending tasks and returns time before next task (in ms).
 * If no task is queued, -1 is returned.
 *
 * @param sched Pointer to the scheduler to use
 *
 * @return Time before next task in ms or -1 if no task is queued
 */
int rinoo_task_driver_run(t_sched *sched)
{
	t_task *task;
	struct timeval tv;
	t_rbtree_node *head;

	XASSERT(sched != NULL, -1);

	while ((head = rbtree_head(&sched->driver.proc_tree)) != NULL) {
		task = container_of(head, t_task, proc_node);
		if (timercmp(&task->tv, &sched->clock, <=)) {
			rinoo_task_unschedule(task);
			rinoo_task_resume(task);
		} else {
			timersub(&task->tv, &sched->clock, &tv);
			return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
		}
	}
	return -1;
}

/**
 * Attempts to stop all pending tasks
 *
 * @param sched Pointer to the scheduler to use
 *
 * @return 0 on success otherwise -1 if an error occurs
 */
int rinoo_task_driver_stop(t_sched *sched)
{
	t_task *task;
	t_rbtree_node *head;

	XASSERT(sched != NULL, -1);
	XASSERT(sched->stop == true, -1);

	while ((head = rbtree_head(&sched->driver.proc_tree)) != NULL) {
		task = container_of(head, t_task, proc_node);
		rinoo_task_unschedule(task);
		rinoo_task_resume(task);
	}
	return 0;
}

/**
 * Returns number of pending tasks.
 *
 * @param sched Pointer to the schedulter to use
 *
 * @return Number of pending tasks.
 */
uint32_t rinoo_task_driver_nbpending(t_sched *sched)
{
	return sched->driver.proc_tree.size;
}

/**
 * Gets current running task.
 *
 * @param sched Pointer to the scheduler to use
 *
 * @return Pointer to the current task
 */
t_task *rinoo_task_driver_getcurrent(t_sched *sched)
{
	return sched->driver.current;
}

/**
 * Create a new task.
 *
 * @param sched sched Pointer to a scheduler to use
 * @param function Routine to call for that task
 * @param arg Routine argument to be passed
 *
 * @return Pointer to the created task, or NULL if an error occurs
 */
t_task *rinoo_task(t_sched *sched, t_task *parent, void (*function)(void *arg), void *arg)
{
	t_task *task;

	XASSERT(sched != NULL, NULL);
	XASSERT(parent != NULL, NULL);
	XASSERT(function != NULL, NULL);

	task = malloc(sizeof(*task));
	if (task == NULL) {
		return NULL;
	}
	task->sched = sched;
	task->scheduled = false;
	task->context.stack.sp = task->stack;
	task->context.stack.size = sizeof(task->stack);
	task->context.link = &parent->context;
	memset(&task->tv, 0, sizeof(task->tv));
	memset(&task->proc_node, 0, sizeof(task->proc_node));
	fcontext(&task->context, function, arg);

#ifdef RINOO_DEBUG
	/* This code avoids valgrind to mix stack switches */
	task->valgrind_stackid = VALGRIND_STACK_REGISTER(task->stack, task->stack + sizeof(task->stack));
#endif /* !RINOO_DEBUG */

	return task;
}

/**
 * Destroy a task.
 *
 * @param task Pointer to the task to destroy
 */
void rinoo_task_destroy(t_task *task)
{
	XASSERTN(task != NULL);

#ifdef RINOO_DEBUG
	VALGRIND_STACK_DEREGISTER(task->valgrind_stackid);
#endif /* !RINOO_DEBUG */
	rinoo_task_unschedule(task);
	free(task);
}

/**
 * Queue a task to be launch asynchronously.
 *
 * @param sched Pointer to the scheduler to use
 * @param function Pointer to the routine function
 * @param arg Argument to be passed to the routine function
 *
 * @return 0 on success, otherwise -1
 */
int rinoo_task_start(t_sched *sched, void (*function)(void *arg), void *arg)
{
	t_task *task;

	task = rinoo_task(sched, &sched->driver.main, function, arg);
	if (task == NULL) {
		return -1;
	}
	rinoo_task_schedule(task, NULL);
	return 0;
}

/**
 * Run a task within the current task.
 * This function will return once the routine returned.
 *
 * @param sched Pointer to the scheduler to use
 * @param function Pointer to the routine function
 * @param arg Argument to be passed to the routine function
 *
 * @return 0 on success, otherwise -1
 */
int rinoo_task_run(t_sched *sched, void (*function)(void *arg), void *arg)
{
	t_task *task;

	task = rinoo_task(sched, sched->driver.current, function, arg);
	if (task == NULL) {
		return -1;
	}
	return rinoo_task_resume(task);
}

/**
 * Resume a task.
 * This function switches to the task stack by calling fcontext_swap.
 *
 * @param task Pointer to the task to run or resume
 *
 * @return 1 if the given task has been executed and is over, 0 if it's been released, -1 if an error occurs
 */
int rinoo_task_resume(t_task *task)
{
	int ret;
	t_task *old;
	t_task_driver *driver;

	XASSERT(task != NULL, -1);

	driver = &task->sched->driver;
	old = driver->current;
	driver->current = task;
	current_task = task;
	ret = fcontext_swap(&old->context, &task->context);
	driver->current = old;
	current_task = old;
	if (ret == 0) {
		/* This task is finished */
		rinoo_task_destroy(task);
	}
	return ret;
}

/**
 * Release execution of a task currently running on a scheduler.
 *
 * @param sched Pointer to the scheduler to use
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_task_release(t_sched *sched)
{
	XASSERT(sched != NULL, -1);

	fcontext_swap(&sched->driver.current->context, &sched->driver.main.context);
	if (sched->stop == true) {
		errno = ECANCELED;
		return -1;
	}
	return 0;
}

/**
 * Schedule a task to be executed at specific time.
 *
 * @param task Pointer to the task to schedule
 * @param tv Pointer to a timeval representing the expected execution time
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_task_schedule(t_task *task, struct timeval *tv)
{
	XASSERT(task != NULL, -1);
	XASSERT(task->sched != NULL, -1);

	if (task->scheduled == true) {
		rbtree_remove(&task->sched->driver.proc_tree, &task->proc_node);
		task->scheduled = false;
	}
	if (tv != NULL) {
		task->tv = *tv;
	} else {
		memset(&task->tv, 0, sizeof(task->tv));
	}
	if (rbtree_put(&task->sched->driver.proc_tree, &task->proc_node) != 0) {
		return -1;
	}
	task->scheduled = true;
	return 0;
}

/**
 * Remove a task which has been scheduled for execution.
 *
 * @param task Pointer to the task to unschedule
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_task_unschedule(t_task *task)
{
	XASSERT(task != NULL, -1);
	XASSERT(task->sched != NULL, -1);

	if (task->scheduled == true) {
		rbtree_remove(&task->sched->driver.proc_tree, &task->proc_node);
		memset(&task->tv, 0, sizeof(task->tv));
		task->scheduled = false;
	}
	return 0;
}

/**
 * Release a task for a given time.
 *
 * @param sched Pointer to the scheduler to use
 * @param ms Release time in milliseconds
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_task_wait(t_sched *sched, uint32_t ms)
{
	struct timeval res;
	struct timeval toadd;

	if (ms == 0) {
		if (rinoo_task_schedule(rinoo_task_driver_getcurrent(sched), NULL) != 0) {
			return -1;
		}
	} else {
		toadd.tv_sec = ms / 1000;
		toadd.tv_usec = (ms % 1000) * 1000;
		timeradd(&sched->clock, &toadd, &res);
		if (rinoo_task_schedule(rinoo_task_driver_getcurrent(sched), &res) != 0) {
			return -1;
		}
	}
	return rinoo_task_release(sched);
}

/**
 * Release a task to be re-scheduled as soon as possible.
 * This can be called by a busy task to give processing back to the scheduler.
 *
 * @param sched Pointer to the scheduler to use
 *
 * @return 0 on success or -1 if an error occurs
 */
int rinoo_task_pause(t_sched *sched)
{
	t_task *task;
	struct timeval tv;

	task = rinoo_task_driver_getcurrent(sched);
	if (task == &sched->driver.main) {
		return 0;
	}
	if (task->scheduled == true) {
		tv = task->tv;
		if (rinoo_task_schedule(task, NULL) != 0) {
			return -1;
		}
		if (rinoo_task_release(sched) != 0) {
			return -1;
		}
		if (rinoo_task_schedule(task, &tv) != 0) {
			return -1;
		}
	} else {
		if (rinoo_task_schedule(task, NULL) != 0) {
			return -1;
		}
		if (rinoo_task_release(sched) != 0) {
			return -1;
		}
	}
	return 0;
}

/**
 * Gets current running task.
 *
 *
 * @return Pointer to the active task or NULL if no task is running
 */
t_task *rinoo_task_self(void)
{
	return current_task;
}
