/**
 * @file   spawn.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Sep 22 16:52:09 2013
 *
 * @brief RiNOO scheduler spawning functions
 *
 *
 */

#include "rinoo/scheduler/module.h"

/**
 * Spawns a number of children schedulers.
 *
 * @param sched Parent scheduler
 * @param count Number of children to create
 *
 * @return 0 on success, otherwise -1
 */
int rinoo_spawn(t_sched *sched, int count)
{
	int i;
	t_sched *child;
	t_thread *thread;

	if (sched->spawns.count == 0) {
		thread = calloc(count, sizeof(*thread));
		if (thread == NULL) {
			return -1;
		}
	} else {
		thread = realloc(sched->spawns.thread, sizeof(*thread) * (sched->spawns.count + count));
		if (thread == NULL) {
			return -1;
		}
	}
	sched->spawns.thread = thread;
	for (i = sched->spawns.count; i < sched->spawns.count + count; i++) {
		child = rinoo_sched();
		if (child == NULL) {
			sched->spawns.count = i;
			return -1;
		}
		child->id = i + 1;
		sched->spawns.thread[i].id = 0;
		sched->spawns.thread[i].sched = child;
	}
	sched->spawns.count = i;
	return 0;
}

/**
 * Destroy all scheduler spawns
 *
 * @param sched
 */
void rinoo_spawn_destroy(t_sched *sched)
{
	if (sched->spawns.thread != NULL) {
		free(sched->spawns.thread);
	}
	sched->spawns.count = 0;
}

/**
 * Get a scheduler spawn from its id.
 * Id 0 returns sched itself.
 *
 * @param sched Scheduler to use
 * @param id Spawn id
 *
 * @return Pointer to the spawn or NULL if an error occured
 */
t_sched *rinoo_spawn_get(t_sched *sched, int id)
{
	if (id < 0 || id > sched->spawns.count) {
		return NULL;
	}
	if (id == 0) {
		return sched;
	}
	return sched->spawns.thread[id - 1].sched;
}

/**
 * Main spawn loop. This function should be executed in a thread.
 *
 * @param sched Scheduler running the loop
 *
 * @return NULL
 */
static void *rinoo_spawn_loop(void *sched)
{
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	return NULL;
}

/**
 * Signal handler called when killing threads.
 * This will stop the running scheduler.
 *
 * @param unused Signal number
 */
static void rinoo_spawn_handler_stop(int unused(sig))
{
	t_sched *sched = rinoo_sched_self();

	if (sched != NULL) {
		rinoo_sched_stop(sched);
	}
}

/**
 * Starts spawns. It creates a thread for each spawn.
 *
 * @param sched Main scheduler
 *
 * @return 0 on success otherwise -1
 */
int rinoo_spawn_start(t_sched *sched)
{
	int i;
	sigset_t oldset;
	sigset_t newset;

	sigemptyset(&newset);
	if (sigaddset(&newset, SIGINT) < 0) {
		return -1;
	}
	if (sigaction(SIGUSR2, &(struct sigaction){ .sa_handler = rinoo_spawn_handler_stop }, NULL) != 0) {
		return -1;
	}
	pthread_sigmask(SIG_BLOCK, &newset, &oldset);
	for (i = 0; i < sched->spawns.count; i++) {
		if (pthread_create(&sched->spawns.thread[i].id, NULL, rinoo_spawn_loop, sched->spawns.thread[i].sched) != 0) {
			pthread_sigmask(SIG_SETMASK, &oldset, NULL);
			return -1;
		}
	}
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);
	return 0;
}

/**
 * Stops all schedule spawns.
 *
 * @param sched Main scheduler
 */
void rinoo_spawn_stop(t_sched *sched)
{
	int i;

	for (i = 0; i < sched->spawns.count; i++) {
		if (sched->spawns.thread[i].id != 0) {
			sched->spawns.thread[i].sched = NULL;
			pthread_kill(sched->spawns.thread[i].id, SIGUSR2);
		}
	}
}

/**
 * Waits for spawns to finish.
 *
 * @param sched Main scheduler
 */
void rinoo_spawn_join(t_sched *sched)
{
	int i;

	for (i = 0; i < sched->spawns.count; i++) {
		if (sched->spawns.thread[i].id != 0) {
			pthread_join(sched->spawns.thread[i].id, NULL);
		}
	}
}
