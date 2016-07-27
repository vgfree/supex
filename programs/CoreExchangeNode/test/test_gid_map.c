#include "core_exchange_node_test.h"
#include "../gid_map.h"

#include <stdio.h>

int test_gid_map()
{
	init_gid_map();
	char    *gid1 = "12345";
	char    *gid2 = "23456";
	int     fd = 2;
	insert_gid_list(fd, gid1);
	insert_gid_list(fd, gid2);
	insert_fd_list(gid1, &fd, 1);
	insert_fd_list(gid2, &fd, 1);

	int     size = 0;
	char    *tested_gid[20] = {};
	find_gid_list(fd, tested_gid, &size);
	int i;

	for (i = 0; i < size; i++) {
		printf("find_gid_list fd:%d, gid:%s.\n", fd, tested_gid[i]);
	}

	int tested_fd_list[10] = {};
	size = find_fd_list(gid1, tested_fd_list);

	for (i = 0; i < size; i++) {
		printf("find_fd_list gid:%s, fd:%d.\n", gid1, tested_fd_list[i]);
	}

	size = find_fd_list(gid2, tested_fd_list);

	for (i = 0; i < size; i++) {
		printf("find_fd_list gid:%s, fd:%d.\n", gid2, tested_fd_list[i]);
	}

	printf("remove fd:%d.\n", fd);

	for (i = 0; i < 2; i++) {
		remove_fd_list(tested_gid[i], &fd, 1);
	}

	remove_gid_list(fd, tested_gid, 2);

	for (i = 0; i < 2; i++) {
		free(tested_gid[i]);
		tested_gid[i] = NULL;
	}

	find_gid_list(fd, tested_gid, &size);

	for (i = 0; i < size; i++) {
		printf("find_gid_list fd:%d, gid:%s.\n", fd, tested_gid[i]);
		free(tested_gid[i]);
	}

	size = find_fd_list(gid1, tested_fd_list);

	for (i = 0; i < size; i++) {
		printf("find_fd_list gid:%s, fd:%d.\n", gid1, tested_fd_list[i]);
	}

	size = find_fd_list(gid2, tested_fd_list);

	for (i = 0; i < size; i++) {
		printf("find_fd_list gid:%s, fd:%d.\n", gid2, tested_fd_list[i]);
	}

	destroy_gid_map();
	return 0;
}

