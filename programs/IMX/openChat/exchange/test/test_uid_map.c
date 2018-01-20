#include "core_exchange_node_test.h"
#include "../uid_map.h"

#include <stdio.h>

int test_uid_map()
{
	exc_uidmap_init();
	char uid[MAX_UID_SIZE] = "123456789";
	char cid[MAX_CID_SIZE] = "1";
	exc_uidmap_set_cid(uid, cid);
	exc_cidmap_set_uid(cid, uid);

	int fd = 1;
	int ok = exc_uidmap_get_cid(uid, cid);
	int result_fd = atoi(cid);
	printf("uid:%s, find fd:%d.\n", uid, result_fd);

	char    test_uid[20] = {};
	int     size = 0;
	exc_cidmap_get_uid(cid, test_uid)
	printf("fd:%d, find uid:%s.\n", fd, test_uid);

	exc_uidmap_del_cid(uid);
	ok = exc_uidmap_get_cid(uid, cid);
	result_fd = atoi(cid);
	exc_cidmap_del_uid(cid);
	exc_cidmap_get_uid(cid, test_uid)

	exc_uidmap_free();
	return 0;
}

