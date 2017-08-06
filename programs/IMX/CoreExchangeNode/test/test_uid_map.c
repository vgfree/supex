#include "core_exchange_node_test.h"
#include "../uid_map.h"

#include <stdio.h>

int test_uid_map()
{
	init_uid_map();
	char    *uid = "123456789";
	int     fd = 1;
	insert_fd(uid, fd);

	int result_fd = find_fd(uid);
	printf("uid:%s, find fd:%d.\n", uid, result_fd);

	char    test_uid[20] = {};
	int     size = 0;
	find_uid(test_uid, &size, fd);
	printf("fd:%d, find uid:%s.\n", fd, test_uid);

	remove_fd(uid);
	find_fd(uid);
	remove_uid(fd);
	find_uid(test_uid, &size, fd);

	destroy_uid_map();
	return 0;
}

