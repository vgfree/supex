/*
 * 版权声明：暂无
 * 文件名称：map_seg.c
 * 创建者   ：王张彦
 * 创建日期：2015/11/18
 * 文件描述：计算前方seg
 * 历史记录：
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include <locale.h>
#include <iconv.h>

#include "db_api.h"
#include "map_seg.h"
#include "seg_cfg.h"
#include "libmini.h"
#include "map_seg_file.h"

map_seg_manager *ptr_manager = NULL;

uint32_t data_load(file_seg_info *p_file_buf)
{
	if (!p_file_buf) {
		return -1;
	}

	if (!p_file_buf->rrid && !p_file_buf->name_id) {
		return 0;
	}

	uint32_t back_count = 0;

	while (back_count < ptr_manager->load_once) {
		file_seg_info *temp = p_file_buf + back_count;

		if (!temp->rrid && !temp->name_id) {
			return back_count;
		}

		int64_t rr_id = temp->rrid;
		int64_t index_offset = (rr_id - 1) / ptr_manager->rrid_buf_long;

		if (index_offset > ptr_manager->index_long - 1) {
			x_printf(E, " rr_id=%ld,index_offset=%ld,out_offset larger than\n", rr_id, index_offset);
			return back_count;
		}

		rrid_buf *ptr_rrid = *(ptr_manager->ptr_index + index_offset);

		if (!ptr_rrid) {
			ptr_rrid = (rrid_buf *)calloc(ptr_manager->rrid_buf_long, sizeof(rrid_buf));

			if (!ptr_rrid) {
				x_printf(E, "ptr_rrid calloc menmory faile\n");
				return back_count;
			}

			*(ptr_manager->ptr_index + index_offset) = ptr_rrid;
		}

		int64_t         rrid_offset = (rr_id - 1) % ptr_manager->rrid_buf_long;
		uint64_t        sgid_count = temp->sgid_count;

		if (rrid_offset > ptr_manager->rrid_buf_long - 1) {
			x_printf(E, "rr_id=%ld,rrid_offset=%ld,in_offset larger than \n", rr_id, rrid_offset);
			return back_count;
		}

		map_seg_info *ptr_sgid = (ptr_rrid + rrid_offset)->ptr_sgid;

		if (!ptr_sgid) {
			ptr_sgid = (map_seg_info *)calloc(sgid_count, sizeof(map_seg_info));

			if (!ptr_sgid) {
				x_printf(E, "ptr_sgid calloc menmory faile\n");
				return back_count;
			}

			(ptr_rrid + rrid_offset)->ptr_sgid = ptr_sgid;
			(ptr_rrid + rrid_offset)->sgid_count = sgid_count;
		}

		uint64_t sgid = temp->sgid;

		if (sgid > sgid_count) {
			x_printf(E, " rr_id= %ld,sgid > sgid_count data error\n", rr_id);
			return back_count;
		}

		(ptr_sgid + sgid - 1)->sgid_rt = temp->sgid_rt;
		(ptr_sgid + sgid - 1)->countyCode = temp->countyCode;
		(ptr_sgid + sgid - 1)->start_grade = temp->start_grade;
		(ptr_sgid + sgid - 1)->end_grade = temp->end_grade;
		(ptr_sgid + sgid - 1)->sgid_id = temp->sgid_id;
		(ptr_sgid + sgid - 1)->sgid_count = sgid_count;
		(ptr_sgid + sgid - 1)->sgid = sgid;
		(ptr_sgid + sgid - 1)->next_sgid = temp->next_sgid;
		(ptr_sgid + sgid - 1)->next_rrid = temp->next_rrid;
		(ptr_sgid + sgid - 1)->rrid = rr_id;
		(ptr_sgid + sgid - 1)->name_id = temp->name_id;
		(ptr_sgid + sgid - 1)->start_name = temp->start_name;
		(ptr_sgid + sgid - 1)->end_name = temp->end_name;
		(ptr_sgid + sgid - 1)->length = temp->length;
		(ptr_sgid + sgid - 1)->start_lon = temp->start_lon;
		(ptr_sgid + sgid - 1)->start_lat = temp->start_lat;
		(ptr_sgid + sgid - 1)->end_lon = temp->end_lon;
		(ptr_sgid + sgid - 1)->end_lat = temp->end_lat;

		if (temp->sg_name && (strlen(temp->sg_name) > 2)) {
			int64_t name_id = temp->name_id;
			int64_t name_index_offset = (name_id - 1) / ptr_manager->name_buf_long;

			if (name_index_offset > ptr_manager->name_index_long - 1) {
				x_printf(E, " name_id= %ld,name_index_offset= %ld,name_index_offset larger than\n", name_id, name_index_offset);
				return back_count;
			}

			char **ptr_name_id = *(ptr_manager->p_name + name_index_offset);

			if (!ptr_name_id) {
				ptr_name_id = (char **)calloc(ptr_manager->name_buf_long, sizeof(char *));

				if (!ptr_name_id) {
					x_printf(E, "ptr_name_id calloc menmory faile\n");
					return back_count;
				}

				*(ptr_manager->p_name + name_index_offset) = ptr_name_id;
			}

			int64_t name_arry_offset = (name_id - 1) % ptr_manager->name_buf_long;

			if (name_arry_offset > ptr_manager->name_buf_long - 1) {
				x_printf(E, "name_id= %ld,name_arry_offset= %ld,name_buf_long larger than\n", name_id, name_arry_offset);
				return back_count;
			}

			char *ptr_name_road = *(ptr_name_id + name_arry_offset);

			if (!ptr_name_road) {
				uint16_t long_name = strlen(temp->sg_name) + 1;
				ptr_name_road = (char *)calloc(long_name, sizeof(char));
				strncpy(ptr_name_road, temp->sg_name, long_name);
				*(ptr_name_id + name_arry_offset) = ptr_name_road;
			}
		}

		back_count++;
	}

	return back_count;
}

uint32_t seg_file_load(char *file_name)
{
	// FILE *p_file = fopen("/home/wang/project/supex/lib/mapdata/libmapsegment/map_seg.data", "rb");
	FILE *p_file = fopen(file_name, "rb");

	if (!p_file) {
		// printf("no \n");
		return -1;
	}

	sgid_file_header *header = (sgid_file_header *)calloc(1, sizeof(sgid_file_header));

	// printf("sizeof(sgid_file_header)=%zi,",sizeof(sgid_file_header));
	if (!header) {
		return -1;
	}

	uint32_t size = fread((void *)header, sizeof(sgid_file_header), 1, p_file);
	assert(size == 1);
	ptr_manager = (map_seg_manager *)calloc(1, sizeof(map_seg_manager));

	if (!ptr_manager) {
		return -1;
	}

	// printf("header->load_once=%zi\n",header->load_once);
	// printf("header->min_index=%zi\n",header->min_index);
	// printf("header->max_index=%zi\n",header->max_index);
	// printf("header->index_long=%zi\n",header->index_long);
	// printf("header->rrid_buf_long=%zi\n",header->rrid_buf_long);
	// printf("header->name_index_long=%zi\n",header->name_index_long);
	// printf("header->name_buf_long=%zi\n",header->name_buf_long);
	// printf("\n");
	// printf("header->p_seg_buf->sgid_rt=%d,",header->p_seg_buf->sgid_rt);
	// sleep(1);

	ptr_manager->load_once = header->load_once;
	ptr_manager->index_long = header->index_long;
	ptr_manager->rrid_buf_long = header->rrid_buf_long;
	ptr_manager->name_index_long = header->name_index_long;
	ptr_manager->name_buf_long = header->name_buf_long;

	ptr_manager->ptr_index = (rrid_buf **)calloc(ptr_manager->index_long, sizeof(rrid_buf *));

	if (!ptr_manager->ptr_index) {
		return -1;
	}

	ptr_manager->p_name = (char ***)calloc(ptr_manager->name_index_long, sizeof(char *));

	if (!ptr_manager->p_name) {
		return -1;
	}

	file_seg_info *p_file_buf = (file_seg_info *)calloc(header->load_once, sizeof(file_seg_info));

	if (!p_file_buf) {
		return -1;
	}

	uint64_t        start_id = header->min_index;
	int             count = 0;

	while (start_id <= header->max_index) {
		size = fread((void *)p_file_buf, sizeof(file_seg_info), header->load_once, p_file);
		// printf("p_file_buf->sgid_rt=%d,",p_file_buf->sgid_rt);
		// printf("p_file_buf->rrid=%zi,",p_file_buf->rrid);
		// sleep(1);
		// printf("\n");

		int back_count = data_load(p_file_buf);
		count = count + back_count;

		if (back_count < header->load_once) {
			break;
		}

		start_id = start_id + header->load_once;
		memset(p_file_buf, 0, header->load_once);
		// break;
	}

	x_printf(D, "load count=%d\n", count);
	return 0;
}

int find_name(uint64_t name_id, char **ptr_name)
{
	if (name_id <= 0) {
		return -1;
	}

	uint64_t name_index_offset = (name_id - 1) / ptr_manager->name_buf_long;

	//	printf("name_index_offset:%d",name_index_offset);
	if (name_index_offset > ptr_manager->name_index_long - 1) {
		x_printf(E, "name_index_offset=%ld,name_id=%ld,find  name_index_offset larger than\n", name_index_offset, name_id);
		return -1;
	}

	char **ptr_index_name = *(ptr_manager->p_name + name_index_offset);

	if (!ptr_index_name) {
		*ptr_name = NULL;
		return 0;
	}

	int64_t name_arry_offset = (name_id - 1) % ptr_manager->name_buf_long;

	//	printf("name_arry_offset:%d",name_arry_offset);
	//	printf("ptr_manager->name_buf_long:%d",ptr_manager->name_buf_long);
	if (name_arry_offset > ptr_manager->name_buf_long - 1) {
		x_printf(E, "name_arry_offset=%ld,find name_buf_long larger than\n", name_arry_offset);
		return -1;
	}

	*ptr_name = *(ptr_index_name + name_arry_offset);
	return 0;
}

int map_seg_query(unsigned int rr_id, unsigned int sgid, back_seg *buf_seg)
{
	if (rr_id < 0) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	// printf("map_seg_query 232\n");
	if (!ptr_manager) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		// printf("map_seg_query 237\n");
		return -1;
	}

	// printf("map_seg_query 238\n");
	if (rr_id > ptr_manager->index_long * ptr_manager->rrid_buf_long) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	if (sgid <= 0) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		sgid = 1;
	}

	uint64_t index_offset = (rr_id - 1) / ptr_manager->rrid_buf_long;

	if (index_offset > ptr_manager->index_long - 1) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		x_printf(E, "out range index_offset\n");
		return -1;
	}

	rrid_buf *ptr_rrid = *(ptr_manager->ptr_index + index_offset);

	if (!ptr_rrid) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	uint64_t rrid_offset = (rr_id - 1) % ptr_manager->rrid_buf_long;

	if (rrid_offset > ptr_manager->rrid_buf_long - 1) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		x_printf(E, "out range rrid_buf_offset\n");
		return -1;
	}

	map_seg_info *ptr_sgid = (ptr_rrid + rrid_offset)->ptr_sgid;

	if (!ptr_sgid) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	if ((ptr_rrid + rrid_offset)->sgid_count < sgid) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	buf_seg->ptr_seg = (ptr_sgid + sgid - 1);

	if (!buf_seg->ptr_seg->rrid || !buf_seg->ptr_seg->sgid) {
		buf_seg->ptr_seg = NULL;
		buf_seg->ptr_name = NULL;
		return -1;
	}

	if (buf_seg->ptr_seg->name_id && (buf_seg->ptr_seg->name_id > 0)) {
		uint64_t        name_id = buf_seg->ptr_seg->name_id;
		uint64_t        name_index_offset = (name_id - 1) / ptr_manager->name_buf_long;

		if (name_index_offset > ptr_manager->name_index_long - 1) {
			x_printf(E, "rr_id=%d,name_index_offset=%ld,name_id=%ld,find  name_index_offset larger than\n", rr_id, name_index_offset, name_id);
			buf_seg->ptr_name = NULL;
		}

		char **ptr_index_name = *(ptr_manager->p_name + name_index_offset);

		if (!ptr_index_name) {
			buf_seg->ptr_name = NULL;
		} else {
			int64_t name_arry_offset = (name_id - 1) % ptr_manager->name_buf_long;

			if (name_arry_offset > ptr_manager->name_buf_long - 1) {
				x_printf(E, " rr_id=%d,name_arry_offset=%ld,find name_buf_long larger than\n", rr_id, name_arry_offset);
				buf_seg->ptr_name = NULL;
			} else {
				buf_seg->ptr_name = *(ptr_index_name + name_arry_offset);
			}
		}
	}

	if (buf_seg->ptr_seg->start_name && (buf_seg->ptr_seg->start_name > 0)) {
		if (find_name(buf_seg->ptr_seg->start_name, &(buf_seg->ptr_start_name)) == -1) {
			x_printf(E, "find start_name error");
			buf_seg->ptr_start_name = NULL;
		}
	} else {
		buf_seg->ptr_start_name = NULL;
	}

	if (buf_seg->ptr_seg->end_name && (buf_seg->ptr_seg->end_name > 0)) {
		if (find_name(buf_seg->ptr_seg->end_name, &(buf_seg->ptr_end_name)) == -1) {
			x_printf(E, "find end_name error");
			buf_seg->ptr_end_name = NULL;
		}
	} else {
		buf_seg->ptr_end_name = NULL;
	}

	return 0;
}

int map_seg_destory()
{
	if (!ptr_manager) {
		return 0;
	}

	if (ptr_manager->ptr_index) {
		int i = 0;

		for (; i < ptr_manager->index_long; i++) {
			rrid_buf *delete_buf = *(ptr_manager->ptr_index + i);

			if (delete_buf) {
				int j = 0;

				for (; j < ptr_manager->rrid_buf_long; j++) {
					if ((delete_buf + j)->ptr_sgid) {
						free((delete_buf + j)->ptr_sgid);
						(delete_buf + j)->ptr_sgid = NULL;
					}
				}

				free(delete_buf);
				*(ptr_manager->ptr_index + i) = NULL;
			}
		}

		free(ptr_manager->ptr_index);
		ptr_manager->ptr_index = NULL;
	}

	if (ptr_manager->p_name) {
		int i = 0;

		for (; i < ptr_manager->name_index_long; i++) {
			char **p_name_buf = *(ptr_manager->p_name + i);

			if (p_name_buf) {
				int j = 0;

				for (; j < ptr_manager->name_buf_long; j++) {
					if (*(p_name_buf + j)) {
						free(*(p_name_buf + j));
						*(p_name_buf + j) = NULL;
					}
				}

				free(p_name_buf);
				*(ptr_manager->p_name + i) = NULL;
			}
		}

		free(ptr_manager->p_name);
		ptr_manager->p_name = NULL;
	}

	free(ptr_manager);
	ptr_manager = NULL;
	return 0;
}

map_iterator_element *map_seg_iterator_init(unsigned int rrid_id, unsigned int sgid, unsigned int end_grade_one_number)
{
	map_iterator_element *back_seg_element = (map_iterator_element *)calloc(1, sizeof(map_iterator_element));

	back_seg_element->rrid = rrid_id;
	back_seg_element->sgid = sgid;
	back_seg_element->next_sgid = 0;
	back_seg_element->next_rrid = 0;
	back_seg_element->sgid_count = 0;
	back_seg_element->surplus_end_grade_one_number = end_grade_one_number;
	back_seg_element->iterator_count = 0;

	return back_seg_element;
}

back_seg *map_seg_iterator_next(map_iterator_element *back_seg_element, back_seg *out)
{
	// printf("rrid_id=%zi,",back_seg_element->rrid);
	// printf("sgid=%d,",back_seg_element->sgid);

	back_seg_element->iterator_count++;

	if (back_seg_element->iterator_count > ITERATOR_COUNT) {
		return NULL;
	}

	if (back_seg_element->surplus_end_grade_one_number == 0) {
		return NULL;
	}

	unsigned int    rrid_id;
	unsigned int    sgid;

	// 表示第一个判断,sgid_count = 0
	if ((back_seg_element->next_sgid == 0) && (back_seg_element->next_rrid == 0) && (back_seg_element->sgid_count == 0)) {
		rrid_id = back_seg_element->rrid;
		sgid = back_seg_element->sgid;
		// printf("*if1;*");
	}

	/*表示sgid还在rrid中*/
	if ((back_seg_element->next_sgid == 0) && (back_seg_element->next_rrid == 0) && (back_seg_element->sgid_count > back_seg_element->sgid)) {
		rrid_id = back_seg_element->rrid;
		sgid = back_seg_element->sgid + 1;
		// printf("*if2;*");
	}

	/*表示sgid位于rrid中末尾，而且next_rrid为0*/
	if ((back_seg_element->next_sgid == 0) && (back_seg_element->next_rrid == 0) && (back_seg_element->sgid_count == back_seg_element->sgid)) {
		// printf("*if3;*");
		return NULL;
	}

	/*表示next_rrid不为0*/
	if ((back_seg_element->next_sgid != 0) && (back_seg_element->next_rrid != 0)) {
		rrid_id = back_seg_element->next_rrid;
		sgid = back_seg_element->next_sgid;
		// printf("*if4;*");
	}

	// printf ("rrid:%zi,sgid:%d,",rrid_id,sgid);
	// map_seg_query(rrid_id,sgid,&out);
	if (map_seg_query(rrid_id, sgid, out) != 0) {
		return NULL;
	}

	map_seg_info *temp = out->ptr_seg;

	if (!temp) {
		return NULL;
	}

	if (temp->end_grade == 1) {
		(back_seg_element->surplus_end_grade_one_number)--;
	}

	back_seg_element->rrid = temp->rrid;
	back_seg_element->sgid = temp->sgid;
	back_seg_element->sgid_count = temp->sgid_count;
	back_seg_element->next_rrid = temp->next_rrid;
	back_seg_element->next_sgid = temp->next_sgid;

	// printf("**temp->sgid_count:%d,",temp->sgid_count);
	// printf("end_grade:%d,",temp->end_grade);
	// printf("(out.ptr)->rrid:%zi,",(out.ptr_seg)->rrid);
	// printf("(next.ptr)->rrid:%zi,",(next.ptr_seg)->rrid);
	return out;
}

int map_seg_iterator_destory(map_iterator_element *back_seg_element)
{
	if (back_seg_element != NULL) {
		free(back_seg_element);
	}

	// map_seg_destory();
	return 0;
}

