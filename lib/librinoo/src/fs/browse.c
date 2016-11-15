/**
 * @file   browse.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Sun Jul 20 17:23:33 2014
 *
 * @brief  Easy directory browsing.
 *
 *
 */

#include "rinoo/fs/module.h"

static int rinoo_fs_stack_push(t_fs_entry *entry, DIR *dirfd, const char *path)
{
	t_fs_directory *directory;

	directory = calloc(1, sizeof(*directory));
	if (directory == NULL) {
		return -1;
	}
	directory->path = strdup(path);
	if (directory->path == NULL) {
		free(directory);
		return -1;
	}
	directory->fd = dirfd;
	list_put(&entry->stack, &directory->stack_node);
	return 0;
}

static int rinoo_fs_stack_pop(t_fs_entry *entry)
{
	t_list_node *node;
	t_fs_directory *directory;

	node = list_pop(&entry->stack);
	if (node == NULL) {
		return -1;
	}
	directory = container_of(node, t_fs_directory, stack_node);
	free(directory->path);
	free(directory);
	return 0;
}

static t_fs_directory *rinoo_fs_stack_head(t_fs_entry *entry)
{
	t_list_node *node;
	t_fs_directory *directory;

	node = list_head(&entry->stack);
	if (node == NULL) {
		return NULL;
	}
	directory = container_of(node, t_fs_directory, stack_node);
	return directory;
}

static void rinoo_fs_stack_destroy_node(t_list_node *node)
{
	t_fs_directory *directory;

	directory = container_of(node, t_fs_directory, stack_node);
	closedir(directory->fd);
	free(directory->path);
	free(directory);
}

static void rinoo_fs_entry_destroy(t_fs_entry *entry)
{
	if (entry == NULL) {
		return;
	}
	if (entry->entry != NULL) {
		free(entry->entry);
	}
	if (entry->path != NULL) {
		buffer_destroy(entry->path);
	}
	list_flush(&entry->stack, rinoo_fs_stack_destroy_node);
	free(entry);
}

static t_fs_entry *rinoo_fs_entry(const char *path)
{
	long size;
	t_fs_entry *entry;

	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) {
		return NULL;
	}
	list(&entry->stack, NULL);
	size = pathconf(path, _PC_NAME_MAX);
	if (size < 0) {
		size = 256;
	}
	size = offsetof(struct dirent, d_name) + size + 1;
	entry->entry = calloc(1, size);
	if (entry->entry == NULL) {
		goto entry_error;
	}
	entry->path = buffer_create(NULL);
	if (entry->path == NULL) {
		goto entry_error;
	}
	if (stat(path, &entry->stat) != 0) {
		goto entry_error;
	}
	if (!S_ISDIR(entry->stat.st_mode)) {
		goto entry_error;
	}
	return entry;
entry_error:
	rinoo_fs_entry_destroy(entry);
	return NULL;
}

int rinoo_fs_browse(const char *path, t_fs_entry **last_entry)
{
	DIR *dirfd;
	t_fs_entry *curentry;
	struct dirent *result;
	t_fs_directory *directory;

	if (last_entry == NULL) {
		return -1;
	}
	curentry = *last_entry;
	if (curentry == NULL) {
		curentry = rinoo_fs_entry(path);
		if (curentry == NULL) {
			return -1;
		}
		dirfd = opendir(path);
		if (dirfd == NULL) {
			goto browse_error;
		}
		if (rinoo_fs_stack_push(curentry, dirfd, path) != 0) {
			closedir(dirfd);
			goto browse_error;
		}
	}
	directory = rinoo_fs_stack_head(curentry);
	if (directory == NULL) {
		goto browse_error;
	}
	for (result = NULL; result == NULL && directory != NULL;) {
		if (readdir_r(directory->fd, curentry->entry, &result) != 0) {
			goto browse_error;
		}
		if (result == NULL) {
			closedir(directory->fd);
			rinoo_fs_stack_pop(curentry);
			directory = rinoo_fs_stack_head(curentry);
			result = NULL;
			continue;
		}
		if (curentry->entry->d_name[0] == '.' && (curentry->entry->d_name[1] == 0 || curentry->entry->d_name[1] == '.')) {
			result = NULL;
			continue;
		}
		buffer_erase(curentry->path, 0);
		buffer_addstr(curentry->path, directory->path);
		if (((char *) buffer_ptr(curentry->path))[curentry->path->size - 1] != '/') {
			buffer_addstr(curentry->path, "/");
		}
		buffer_addstr(curentry->path, curentry->entry->d_name);
		buffer_addnull(curentry->path);
		if (stat(buffer_ptr(curentry->path), &curentry->stat) != 0) {
			/* Try next entry */
			result = NULL;
			continue;
		}
		if (S_ISDIR(curentry->stat.st_mode)) {
			dirfd = opendir(buffer_ptr(curentry->path));
			if (dirfd == NULL) {
				/* Try next entry */
				result = NULL;
				continue;
			}
			if (rinoo_fs_stack_push(curentry, dirfd, buffer_ptr(curentry->path)) != 0) {
				goto browse_error;
			}
		}
	}
	if (result == NULL && directory == NULL) {
		/* End of browsing */
		rinoo_fs_entry_destroy(curentry);
		*last_entry = NULL;
		return 0;
	}
	*last_entry = curentry;
	return 0;
browse_error:
	if (curentry != NULL) {
		rinoo_fs_entry_destroy(curentry);
	}
	return -1;
}
