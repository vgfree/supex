/**
 * @file   browse.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Sun Jul 20 17:23:33 2014
 *
 * @brief  Header file for easy directory browsing.
 *
 *
 */

#ifndef RINOO_FS_BROWSE_H_
#define RINOO_FS_BROWSE_H_

typedef struct s_fs_directory {
	DIR *fd;
	char *path;
	t_list_node stack_node;
} t_fs_directory;

typedef struct s_fs_entry {
	t_buffer *path;
	struct stat stat;
	struct dirent *entry;
	t_list stack;
} t_fs_entry;

int rinoo_fs_browse(const char *path, t_fs_entry **last_entry);

#endif /* !RINOO_FS_BROWSE_H_ */
