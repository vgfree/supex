#include "core_exchange_node_test.h"
#include "../gid_map.h"

#include <stdio.h>

void each_cid_print(char cid[MAX_CID_SIZE], size_t idx, void *usr)
{
	printf("find gid:%s, cid:%s.\n", usr, cid);
}

void each_gid_print(char gid[MAX_GID_SIZE], size_t idx, void *usr)
{
	printf("find cid:%s, gid:%s.\n", usr, gid);
}

int test_gid_map()
{
	init_gid_map();
	char    *gid1 = "12345";
	char    *gid2 = "23456";
	int     fd = 2;
	char *cid = "2";
	exc_cidmap_add_gid(cid, gid1);
	exc_cidmap_add_gid(cid, gid2);
	exc_gidmap_add_cid(gid1, cid);
	exc_gidmap_add_cid(gid2, cid);

	exc_cidmap_get_gid(cid, each_gid_print, cid);

	exc_gidmap_get_cid(gid1, each_cid_print, gid1);

	exc_gidmap_get_cid(gid2, each_cid_print, gid2);

	printf("remove fd:%d.\n", fd);

		char cid[MAX_CID_SIZE] = {};
		snprintf(cid, sizeof(cid), "%d", fd);
	for (i = 0; i < 2; i++) {
		exc_gidmap_rem_cid(tested_gid[i], cid);
	}

		exc_cidmap_rem_gid(cid, tested_gid[0]);
		exc_cidmap_rem_gid(cid, tested_gid[1]);

	for (i = 0; i < 2; i++) {
		free(tested_gid[i]);
		tested_gid[i] = NULL;
	}

	size = 20;
	exc_cidmap_get_gid(cid, each_gid_print, cid);
	exc_gidmap_get_cid(gid1, each_cid_print, gid1);
	exc_gidmap_get_cid(gid2, each_cid_print, gid2);

	destroy_gid_map();
	return 0;
}

