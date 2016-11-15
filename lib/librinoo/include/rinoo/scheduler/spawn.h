/**
 * @file   spawn.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Sep 22 16:58:02 2013
 *
 * @brief  RiNOO scheduler spawning functions
 *
 *
 */

#ifndef RINOO_SCHEDULER_SPAWN_H_
#define RINOO_SCHEDULER_SPAWN_H_

/* Defined in scheduler.h */
struct s_sched;

typedef struct s_thread {
	pthread_t id;
	struct s_sched *sched;
} t_thread;

typedef struct s_sched_spawns {
	int count;
	t_thread *thread;
} t_sched_spawns;

int rinoo_spawn(struct s_sched *sched, int count);
void rinoo_spawn_destroy(struct s_sched *sched);
struct s_sched *rinoo_spawn_get(struct s_sched *sched, int id);
int rinoo_spawn_start(struct s_sched *sched);
void rinoo_spawn_stop(struct s_sched *sched);
void rinoo_spawn_join(struct s_sched *sched);

#endif /* !RINOO_SCHEDULER_SPAWN_H_ */
