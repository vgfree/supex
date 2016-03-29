#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#define MYSQL_MAX 32
#include "../map_seg.h"
#include "libmini.h"
int main()
{
	//	map_seg_cfg p;

	// seg_load_cfg_mysql("SEG_conf.json", &p);
	seg_file_load("map_seg.data");
	back_seg back_p;

	int     max_rrid = 5962082;
	int     start_rrid = 1;
	int     count = 0;

	for (; start_rrid < max_rrid; start_rrid++) {
		int     max_sgid = 228;
		int     start_sgid = 1;

		for (; start_sgid < max_sgid; start_sgid++) {
			map_seg_query(start_rrid, start_sgid, &back_p);
			map_seg_info *temp = back_p.ptr_seg;

			if (!temp) {
				break;
			}

			//	char *p_name = back_p.ptr_name;
			count++;
			// printf("sgid_rt=%d; ", temp->sgid_rt);
			// printf("sgid_count=%d; ", temp->sgid_count);
			// printf("start_grade=%d; ", temp->start_grade);
			// printf("end_grade=%d; ", temp->end_grade);
			// printf("sgid_id=%d; ", temp->sgid_id);
			printf("sgid=%d; ", temp->sgid);
			// printf("next_sgid=%d; ", temp->next_sgid);
			// printf("next_rrid=%ld; ", temp->next_rrid);
			printf("rrid=%ld; ", temp->rrid);

			// printf("id=%ld; ", temp->name_id);
			// printf("start_name=%ld; ", temp->start_name);
			// printf("end_name=%ld; ", temp->end_name);
			// printf("length=%ld; ", temp->length);
			// printf("start_lon=%.10f; ", temp->start_lon);
			// printf("start_lat=%.10f; ", temp->start_lat);
			// printf("end_lon=%.10f; ", temp->end_lon);
			// printf("end_lat=%.10f; ", temp->end_lat);
			if (back_p.ptr_name && (strlen(back_p.ptr_name) > 2)) {
				printf("name=%s; ", back_p.ptr_name);
			}

			if (back_p.ptr_start_name && (strlen(back_p.ptr_start_name) > 2)) {
				printf("start_name=%s; ", back_p.ptr_start_name);
			}

			if (back_p.ptr_end_name && (strlen(back_p.ptr_end_name) > 2)) {
				printf("end_name=%s; ", back_p.ptr_end_name);
			}

			printf("\n");
		}
	}

	printf("%d\n", count);

	map_seg_destory();
	return 0;
}

