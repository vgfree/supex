/**
 * @file   rinoo_fs_browse.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Mon Jul 21 18:21:24 2014
 *
 * @brief  Test file for rinoo fs browse.
 *
 *
 */

#include "rinoo/rinoo.h"

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_fs_entry *entry = NULL;

	while (rinoo_fs_browse(".", &entry) == 0 && entry != NULL) {
		rinoo_log("%s", buffer_ptr(entry->path));
	}
	XPASS();
}
