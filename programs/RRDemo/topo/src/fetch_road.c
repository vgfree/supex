/*
 *  fetch_road.c
 *
 *  Created on: Feb 25, 2016
 *  Author: shu
 * */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fetch_road.h"
#include "topo_com.h"
#include "topo.h"
#include "topo_api.h"

static int random_by_range(int x, int y)
{
	if ((x <= 0) || (y <= 0)) {
		printf("failed! random parameter\n");
		return -1;
	}

	if (x == y) {
		return 0;
	}

	srandom((int)time(0));
	int range = abs(y - x + 1);
	return random() % range;
}

static uint64_t topo_call(uint64_t roadid, int max, TOPO_CALLBACK topo_cb)
{
	struct query_args       info = {};
	uint64_t                slot[max];
	int                     i = 0;

	memset(&info, 0, sizeof(info));
	info.idx = roadid;
	info.buf = slot;
	info.peak = max;

	topo_cb(&info);

	int randroad = random_by_range(1, info.size);

	if (randroad < 0) {
		printf("get random road failed\n");
		return -1;
	} else {
		for (i = 0; i < info.size; i++) {
			printf("r%d: %d", i, slot[i]);
		}

		printf("count:%d rand:%d road:%d\n", info.size, randroad, slot[randroad]);
		return slot[randroad];
	}
}

int get_length_of_road(uint64_t roadid)
{
	TP_LINE_OBJ *p_line = topo_pull_line(roadid);

	if (NULL == p_line) {
		printf("failed! road is null roadID:%d\n", roadid);
		return -1;
	}

	printf("%d -- %d length:%d\n", p_line->from_node->id, p_line->goto_node->id, p_line->length);
	return p_line->length;
}

uint64_t get_nextrandomroad_of_road(uint64_t roadid)
{
	return topo_call(roadid, MAX_ONE_NODE_OWN_LINE_COUNT, get_export_road_by_road);
}

