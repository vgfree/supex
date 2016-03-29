/*版权声明：无
 * 文件名称：map_line.c
 * 创建者：王张彦
 * 创建日期：2015.10.19
 * 文件描述：存储line信息，查询line的信息
 *历史记录：无
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "map_line.h"
#include "map_line_file.h"
#include "pmr_cfg.h"
#include "db_api.h"

#define  LOAD_STEP      10000
#define  ERROR_DATA     0
map_line_manager *ptr_line_manager = NULL;

int map_line_load(char *line_file)
{
	if (NULL == line_file) {
		return -1;
	}

        ptr_line_manager = (map_line_manager *)calloc(1, sizeof(struct map_line_manager));

        if (NULL == ptr_line_manager) {
		return -1;
	}

        map_line_load_file(line_file, ptr_line_manager, LOAD_STEP);

	return 0;
}

/*
 * 名    称：map_line_query
 * 功    能：查询line_id信息
 * 参    数：line_id，
 * 返 回 值：指针
 */
map_line_info *map_line_query(unsigned int line_id)
{
        if ((line_id <= 0) || (ptr_line_manager->max_line_id < line_id) || (line_id < ptr_line_manager->min_line_id)) {
		return NULL;
	}

        if (NULL == ptr_line_manager) {
		return NULL;
	}

        int64_t temp_line_id = line_id - ptr_line_manager->min_line_id;

	if (temp_line_id < 0) {
		return NULL;
	}

	int64_t         arry_offset = temp_line_id / LOAD_STEP;
        map_line_info   **back_temp = ptr_line_manager->ptr_arry + arry_offset;

	if (NULL == *back_temp) {
		return NULL;
	}

	int64_t         ptr_offset = temp_line_id % LOAD_STEP;
	map_line_info   *back_ptr = *back_temp + ptr_offset;

	if (NULL == back_ptr) {
		return NULL;
	}

	if ((ERROR_DATA == back_ptr->line_id) && (ERROR_DATA == back_ptr->rr_id) && (ERROR_DATA == back_ptr->sgid)) {
		return NULL;
	}

	return back_ptr;
}

/*
 * 名    称：map_line_destory
 * 功    能：释放内存
 * 参    数：NULL
 * 返 回 值：0
 */
int map_line_destory()
{
        if (NULL == ptr_line_manager) {
		return 0;
	}

        map_line_manager_destory(ptr_line_manager);
        free(ptr_line_manager);
        ptr_line_manager = NULL;
	return 0;
}

