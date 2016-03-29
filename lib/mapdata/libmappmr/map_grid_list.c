#include "map_grid_list.h"
#include "map_errno.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int32_t map_grid_list_init(map_grid_list *p_grid_list, uint32_t max_size, uint32_t step_len)
{
	if (!p_grid_list || (step_len == 0)) {
		return ERR_PMR_PARAMETER;
	}

	p_grid_list->max_size = max_size;
	p_grid_list->step_len = step_len;
	uint32_t count = max_size / step_len;
	count = (max_size % step_len == 0) ? count : (count + 1);
	p_grid_list->list_len = count;

	p_grid_list->p_list = (map_grid_info **)calloc(count, sizeof(map_grid_info *));
	assert(p_grid_list->p_list);
	int i;

	for (i = 0; i < count; i++) {
		map_grid_info *p_infos = (map_grid_info *)calloc(step_len, sizeof(map_grid_info));
		assert(p_infos);
		p_grid_list->p_list[i] = p_infos;
	}

	return 0;
}

/*根据mapgrid id编号,
 *   index：数据库表中id的值
 */
static map_grid_info *find_map_grid(map_grid_list *p_grid_list, uint32_t index)
{
	if (!p_grid_list) {
		return NULL;
	}

	if (index > p_grid_list->max_size) {
		return NULL;
	}

	uint32_t        step_row = index / p_grid_list->step_len;
	uint32_t        step_col = index % p_grid_list->step_len;

	map_grid_info *p_index1 = p_grid_list->p_list[step_row];

	if (!p_index1) {
		return NULL;
	} else {
		return p_index1 + step_col;
	}
}

/*添加grid到内存中
 *   index：数据库表中id的值
 */
map_grid_info *map_grid_list_add(map_grid_list *p_grid_list, map_grid_info *p_info, uint32_t index)
{
	map_grid_info *p_res = find_map_grid(p_grid_list, index);

	if (!p_res) {
		return NULL;
	}

	if (index > p_grid_list->max_size) {
		x_printf(E, "map_grid_list_add,ERR_PMR_OUT_RANGE, index:%d, max:%d\n", index, p_grid_list->max_size);
		return NULL;
	}

	p_res->grid_lat = p_info->grid_lat;
	p_res->grid_lon = p_info->grid_lon;
	p_res->line_size = p_info->line_size;
	p_res->max_size = p_info->max_size;
	p_res->p_lines = p_info->p_lines;

	return p_res;
}

int32_t map_grid_index_list_init(map_grid_index_list *p_index_list, uint32_t max_size, uint32_t step_len)
{
	if (!p_index_list || (step_len == 0)) {
		return ERR_PMR_PARAMETER;
	}

	p_index_list->max_size = max_size;
	p_index_list->step_len = step_len;
	uint32_t count = max_size / step_len;
	count = (max_size % step_len == 0) ? count : (count + 1);
	p_index_list->list_len = count;

	p_index_list->p_list = (map_grid_info ***)calloc(count, sizeof(map_grid_info * *));
	assert(p_index_list->p_list);
	int i;

	for (i = 0; i < count; i++) {
		map_grid_info **p_infos = (map_grid_info **)calloc(step_len, sizeof(map_grid_info *));
		assert(p_infos);
		p_index_list->p_list[i] = p_infos;
	}

	return 0;
}

int32_t map_grid_index_list_add(map_grid_index_list *p_index_list, map_grid_info *p_info, uint32_t index)
{
	if (!p_index_list) {
		return ERR_PMR_PARAMETER;
	}

	if (index > p_index_list->max_size) {
		x_printf(E, "map_grid_index_list_add,ERR_PMR_OUT_RANGE, index:%d, max:%d\n", index, p_index_list->max_size);
		return ERR_PMR_OUT_RANGE;
	}

	uint32_t        step_row = index / p_index_list->step_len;
	uint32_t        step_col = index % p_index_list->step_len;

	map_grid_info **p_index1 = p_index_list->p_list[step_row];

	if (!p_index1) {
		return -1;
	}

	p_index1[step_col] = p_info;

	return 0;
}

/*根据经纬度索引查找mapgrid,
 * index : 网格中经纬度索引
 */
map_grid_info *find_map_grid_index(map_grid_index_list *p_index_list, uint32_t index)
{
	if (!p_index_list) {
		return NULL;
	}

	if (index > p_index_list->max_size) {
		return NULL;
	}

	uint32_t        step_row = index / p_index_list->step_len;
	uint32_t        step_col = index % p_index_list->step_len;

	map_grid_info **p_index1 = p_index_list->p_list[step_row];

	if (!p_index1) {
		return NULL;
	}

	return p_index1[step_col];
}

