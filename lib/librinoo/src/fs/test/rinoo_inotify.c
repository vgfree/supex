/**
 * @file   rinoo_socket_inotify.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Wed Jul 16 22:18:10 2014
 *
 * @brief  Test file for rinoo inotify.
 *
 *
 */

#include "rinoo/rinoo.h"

#define TEST_DIRECTORY	"/tmp/.inotify.test/"
#define NB_EVENT	20

static int nb_create = 0;
static int nb_create_event = 0;
static int nb_rm = 0;
static int nb_rm_event = 0;

void event_generator(void *sched)
{
	int i;
	int fd;
	char path[100];

	rinoo_task_wait(sched, 200);
	for (i = 0; i < NB_EVENT / 2; i++) {
		snprintf(path, sizeof(path), TEST_DIRECTORY ".inotify.XXXXXX");
		fd = mkstemp(path);
		close(fd);
		rinoo_log("Event generator: file created.");
		nb_create++;
		rinoo_task_wait(sched, 200);
		unlink(path);
		nb_rm++;
		rinoo_log("Event generator: file removed.");
		rinoo_task_wait(sched, 200);
	}
}

void check_file(void *sched)
{
	int i;
	t_inotify *inotify;
	t_inotify_event *event;

	inotify = rinoo_inotify(sched);
	rinoo_inotify_add_watch(inotify, "/tmp", INOTIFY_CREATE | INOTIFY_DELETE, true);
	for (i = 0; i < NB_EVENT && (event = rinoo_inotify_event(inotify)) != NULL; i++) {
		if (event->type & INOTIFY_CREATE) {
			rinoo_log("File created.");
			nb_create_event++;
		} else if (event->type & INOTIFY_DELETE) {
			rinoo_log("File deleted.");
			nb_rm_event++;
		}
	}
	XTEST(nb_create_event == nb_create);
	XTEST(nb_rm_event == nb_rm);
	rinoo_inotify_destroy(inotify);
}

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_sched *sched;

	mkdir(TEST_DIRECTORY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, check_file, sched) == 0);
	XTEST(rinoo_task_start(sched, event_generator, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	rmdir(TEST_DIRECTORY);
	XPASS();
}
