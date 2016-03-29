#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#define MYSQL_MAX 32
#include "../map_seg.h"
int main()
{
	seg_file_load("map_seg.data");

	map_iterator_element *back_seg_element = map_seg_iterator_init(548531, 2, 5);

	back_seg out;

	while (map_seg_iterator_next(back_seg_element, &out)) {
		printf("back_seg_element->rrid:%zi,", back_seg_element->rrid);
		printf("back_seg_element->next_rrid:%zi,", back_seg_element->next_rrid);
		printf("back_seg_element->next_sgid:%d,", back_seg_element->next_sgid);
		// printf("out->rrid:%zi,",(out.ptr_seg)->rrid);
		printf("out_p->rrid:%zi,", (out.ptr_seg)->rrid);
		printf("outP->sgid:%d,", (out.ptr_seg)->sgid);
		printf("outP->sgid_count:%d,", (out.ptr_seg)->sgid_count);
		printf("outP:%p,", out.ptr_seg);
		printf("outP->ptr_name:%s,", out.ptr_name);
		printf("outP->ptr_start_name:%s,", out.ptr_start_name);
		printf("outP->ptr_end_name:%s", out.ptr_end_name);
		printf("\n");
	}

	printf("iterator_count:%d\n", back_seg_element->iterator_count);
	map_seg_iterator_destory(back_seg_element);

	map_seg_destory();

	return 0;
}

