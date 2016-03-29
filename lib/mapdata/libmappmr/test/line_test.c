#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include <string.h>
#include <assert.h>
#include "../map_line.h"
#include "libmini.h"

int main(int argc, char **argv)
{
	map_line_load("../../tools/map_line.data");
	int64_t i = 1;
	int64_t pre_id = 0;

	for (; i <= 300000; i++) {
		map_line_info *ptr = map_line_query(i);

		if (NULL == ptr) {
			x_printf(D, "error\n  ");
			break;
		}

		if (1 == i) {
			pre_id = (ptr)->line_id;
		} else {
			if (1 == (ptr)->line_id - pre_id) {
				pre_id = (ptr)->line_id;
			} else {
				x_printf(D, "next error\n  ");
				break;
			}
		}

		x_printf(D, "line_id = %d  ", (ptr)->line_id);
		x_printf(D, "start_lon = %-10f ", (ptr)->start_lon);
		x_printf(D, "start_lat = %-10f ", (ptr)->start_lat);
		x_printf(D, "end_lon = %-10f ", (ptr)->end_lon);
		x_printf(D, "end_lat = %-10f ", (ptr)->end_lat);
		x_printf(D, "dir = %-5d ", (ptr)->dir);
		x_printf(D, "sgid = %d ", (ptr)->sgid);
		x_printf(D, "rrid = %-5d ", (ptr)->rr_id);
		x_printf(D, "rt = %-5d\n", (ptr)->rt);

		x_printf(D, "\n");
	}

	map_line_destory();

	return 0;
}

