/**
 * @file   inotify.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Tue Jul 15 19:35:23 2014
 *
 * @brief  Implementation of inotify for rinoo.
 *
 *
 */

#include "rinoo/fs/module.h"

t_inotify *rinoo_inotify(t_sched *sched)
{
	int fd;
	t_inotify *notify;

	fd = inotify_init1(IN_NONBLOCK);
	if (fd < 0) {
		return NULL;
	}
	notify = calloc(1, sizeof(*notify));
	if (notify == NULL) {
		close(fd);
		return NULL;
	}
	notify->event.path = buffer_create(NULL);
	if (notify->event.path == NULL) {
		close(fd);
		free(notify);
		return NULL;
	}
	notify->node.fd = fd;
	notify->node.sched = sched;
	return notify;
}

void rinoo_inotify_destroy(t_inotify *notify)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(notify->watches); i++) {
		if (notify->watches[i] != NULL) {
			rinoo_inotify_rm_watch(notify, notify->watches[i]);
		}
	}
	rinoo_sched_remove(&notify->node);
	close(notify->node.fd);
	buffer_destroy(notify->event.path);
	free(notify);
}

t_inotify_watch *rinoo_inotify_add_watch(t_inotify *inotify, const char *path, t_inotify_type type, bool recursive)
{
	int wd;
	int nb;
	t_fs_entry *entry;
	t_inotify_watch *watch;

	if (inotify->nb_watches >= ARRAY_SIZE(inotify->watches)) {
		return NULL;
	}
	wd = inotify_add_watch(inotify->node.fd, path, type);
	if (wd < 0) {
		return NULL;
	}
	if ((unsigned int) wd >= ARRAY_SIZE(inotify->watches)) {
		inotify_rm_watch(inotify->node.fd, wd);
		return NULL;
	}
	watch = calloc(1, sizeof(*watch));
	if (watch == NULL) {
		inotify_rm_watch(inotify->node.fd, wd);
		return NULL;
	}
	watch->wd = wd;
	watch->path = strdup(path);
	if (watch->path == NULL) {
		inotify_rm_watch(inotify->node.fd, wd);
		free(watch);
		return NULL;
	}
	if (recursive) {
		nb = 0;
		entry = NULL;
		while (rinoo_fs_browse(path, &entry) == 0 && entry != NULL) {
			nb++;
			if (nb > 100) {
				nb = 0;
				rinoo_task_pause(inotify->node.sched);
			}
			if (S_ISDIR(entry->stat.st_mode)) {
				rinoo_inotify_add_watch(inotify, buffer_ptr(entry->path), type, false);
			}
		}
	}
	inotify->watches[wd] = watch;
	inotify->nb_watches++;
	return watch;
}

int rinoo_inotify_rm_watch(t_inotify *inotify, t_inotify_watch *watch)
{
	inotify_rm_watch(inotify->node.fd, watch->wd);
	inotify->watches[watch->wd] = NULL;
	inotify->nb_watches--;
	free(watch->path);
	free(watch);
	return 0;
}

static int rinoo_inotify_waitio(t_inotify *inotify)
{
	inotify->io_calls++;
	if (inotify->io_calls > 10) {
		inotify->io_calls = 0;
		if (rinoo_task_pause(inotify->node.sched) != 0) {
				return -1;
		}
	}
	return 0;
}

t_inotify_event *rinoo_inotify_event(t_inotify *inotify)
{
	ssize_t ret;
	struct inotify_event *ievent;

	if (rinoo_inotify_waitio(inotify) != 0) {
		return NULL;
	}
	errno = 0;
	while ((ret = read(inotify->node.fd, inotify->read_buffer, sizeof(inotify->read_buffer))) < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return NULL;
		}
		inotify->io_calls = 0;
		if (rinoo_sched_waitfor(&inotify->node, RINOO_MODE_IN) != 0) {
			return NULL;
		}
		errno = 0;
	}
	ievent = (struct inotify_event *) inotify->read_buffer;
	inotify->event.type = ievent->mask;
	inotify->event.watch = inotify->watches[ievent->wd];
	buffer_erase(inotify->event.path, 0);
	buffer_addstr(inotify->event.path, inotify->event.watch->path);
	if (ievent->name[0] != 0) {
		buffer_addstr(inotify->event.path, "/");
		buffer_addstr(inotify->event.path, ievent->name);
	}
	buffer_addnull(inotify->event.path);
	return &inotify->event;
}
