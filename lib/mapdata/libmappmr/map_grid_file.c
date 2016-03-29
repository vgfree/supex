/*
 * 版权声明：暂无
 * 文件名称：map_grid_file.c
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/20
 * 文件描述：load data from file
 * 历史记录：
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "map_grid_file.h"
#include "map_errno.h"
#include "map_grid_list.h"
#include "map_utils.h"

/*网格数据读写文件的文件头*/
typedef struct grid_file_header
{
	uint16_t        lon_size;
	uint16_t        lat_size;
	int32_t         lon_begin;
	int32_t         lat_begin;
	uint32_t        min_id;
	uint32_t        max_id;
} grid_file_header;

/*保存网格与line关系数据，写入文件结构体*/
typedef struct file_grid_info
{
	uint32_t        id;
	int32_t         grid_lon;
	int32_t         grid_lat;
	uint32_t        num;	/*用于保存lineID或者line count*/
} file_grid_info;

typedef struct file_grid_buffer
{
	file_grid_info  *p_buf;
	uint32_t        size;
	uint32_t        start_id;
	int32_t         max_lon;
	int32_t         min_lon;
	int32_t         max_lat;
	int32_t         min_lat;
} file_grid_buffer;

static int32_t grid_count_load_file(char *grid_count_file, map_grid_manager *p_manage, uint32_t step_len)
{
	uint32_t        count = 0;
	FILE            *p_file = fopen(grid_count_file, "rb");	// 二进制模式打开

	assert(p_file);

	// 读文件头
	grid_file_header        header = {};
	uint32_t                size = fread((void *)&header, sizeof(grid_file_header), 1, p_file);
	assert(size == 1);

	if ((header.max_id <= 0) || (header.min_id <= 0)) {
		x_printf(E, "load map line file error, min_id:%d, max_id:%d\n", header.min_id, header.max_id);
		return -1;
	}

	p_manage->lon_begin = header.lon_begin;
	p_manage->lon_size = header.lon_size;
	p_manage->lat_begin = header.lat_begin;
	p_manage->lat_size = header.lat_size;

	uint32_t max_size = header.lon_size * header.lat_size;
	map_grid_list_init(&p_manage->grid_list, header.max_id, step_len);
	map_grid_index_list_init(&p_manage->index_list, max_size, step_len);

	// 分配预读buffer
	uint32_t        index_count = header.max_id / step_len;
	int             last_count = header.max_id % step_len;
	index_count = (last_count == 0) ? index_count : index_count + 1;
	file_grid_info *p_buf = (file_grid_info *)calloc(step_len, sizeof(file_grid_info));

	// 分段与读文件到内存中
	int index;

	for (index = 0; index < index_count; index++) {
		int load_once = step_len;

		if ((index == (index_count - 1)) && (last_count != 0)) {/*最后一次，取剩余数量*/
			load_once = last_count;
		}

		uint32_t ret = fread((void *)p_buf, sizeof(file_grid_info), load_once, p_file);

		if (ret <= 0) {
			continue;
		}

		assert(ret == load_once);

		/*遍历每一个元素，分配存储lineID的内存*/
		int i;

		for (i = 0; i < ret; i++) {
			file_grid_info  *p_info = p_buf + i;
			map_grid_info   grid;
			grid.grid_lon = p_info->grid_lon;
			grid.grid_lat = p_info->grid_lat;

			/*判断是否超出网格范围*/
			if ((grid.grid_lon < p_manage->lon_begin) ||
				((grid.grid_lon - p_manage->lon_begin) / 5 >= p_manage->lon_size) ||
				(grid.grid_lat < p_manage->lat_begin) ||
				((grid.grid_lat - p_manage->lat_begin) / 5 >= p_manage->lat_size)) {
				x_printf(E, "out of range, lon:%d,lat:%d\n", grid.grid_lon, grid.grid_lat);
				continue;
			}

			grid.max_size = p_info->num;
			grid.line_size = 0;
			grid.p_lines = (uint32_t *)calloc(p_info->num, sizeof(uint32_t));
			map_grid_info *p_grid_info = map_grid_list_add(&p_manage->grid_list, &grid, p_info->id);

			if (!p_grid_info) {
				continue;
			}

			int32_t grid_index = (p_grid_info->grid_lat - p_manage->lat_begin) / 5 * p_manage->lon_size + (p_grid_info->grid_lon - p_manage->lon_begin) / 5;
			map_grid_index_list_add(&p_manage->index_list, p_grid_info, grid_index);
		}

		count += ret;
                //x_printf(D, "load map gird count, num:%d\n", count);
	}

	fclose(p_file);
	return count;
}

static int32_t grid_line_load_file(char *grid_count_file, map_grid_manager *p_manage, uint32_t step_len)
{
	uint32_t        count = 0;
	FILE            *p_file = fopen(grid_count_file, "rb");	// 二进制模式打开

	assert(p_file);

	// 读文件头
	grid_file_header        header = {};
	uint32_t                size = fread((void *)&header, sizeof(grid_file_header), 1, p_file);
	assert(size == 1);

	if ((header.max_id <= 0) || (header.min_id <= 0)) {
		x_printf(E, "load map line file error, min_id:%d, max_id:%d\n", header.min_id, header.max_id);
		return -1;
	}

	// 分配预读buffer
	uint32_t        index_count = header.max_id / step_len;
	int             last_count = header.max_id % step_len;
	index_count = (last_count == 0) ? index_count : index_count + 1;
	file_grid_info *p_buf = (file_grid_info *)calloc(step_len, sizeof(file_grid_info));

	// 分段与读文件到内存中
	int index;

	for (index = 0; index < index_count; index++) {
		int load_once = step_len;

		if ((index == (index_count - 1)) && (last_count != 0)) {/*最后一次，取剩余数量*/
			load_once = last_count;
		}

		uint32_t ret = fread((void *)p_buf, sizeof(file_grid_info), load_once, p_file);

		if (ret <= 0) {
			continue;
		}

		assert(ret == load_once);

		/*遍历每一个元素，分配存储lineID的内存*/
		int i;

		for (i = 0; i < ret; i++) {
			file_grid_info  *p_info = p_buf + i;
			int32_t         grid_index = (p_info->grid_lat - p_manage->lat_begin) / 5 * p_manage->lon_size + (p_info->grid_lon - p_manage->lon_begin) / 5;
			map_grid_info   *p_grid_info = find_map_grid_index(&p_manage->index_list, grid_index);
                        //x_printf(D, "load line grid:%d&%d, index:%d,ptr:%p\n", p_info->grid_lon, p_info->grid_lat, grid_index, p_grid_info);

			if (!p_grid_info) {
				continue;
			}

			if (p_info->num > header.max_id) {
                                //x_printf(I, "out of range, lineID:%d, max:%d\n", p_info->num, header.max_id);
				continue;
			}

			if (p_grid_info->max_size < (p_grid_info->line_size + 1)) {
                                //x_printf(I, "out of range linecont, lineID:%d, max:%d\n", p_grid_info->line_size, p_grid_info->max_size);
				continue;
			}

			p_grid_info->p_lines[p_grid_info->line_size] = p_info->num;
			p_grid_info->line_size++;
		}

		count += ret;
                //x_printf(D, "load map gird count, num:%d\n", count);
	}

	fclose(p_file);
	return count;
}

/*加载网格文件*/
int32_t map_grid_load_file(char *grid_count_file, char *grid_line_file, map_grid_manager *p_manage, uint32_t step_len)
{
	if (!grid_count_file || !grid_line_file || !p_manage || (step_len <= 0)) {
		return ERR_PMR_PARAMETER;
	}

	grid_count_load_file(grid_count_file, p_manage, step_len);
	grid_line_load_file(grid_line_file, p_manage, step_len);

	return 0;
}

// 处理数据库查询结果回调函数
static int64_t fetch_map_grid_cb(MYSQL_RES *res, void *args)
{
	if (!res || !args) {
		return -1;
	}

	int64_t row = mysql_num_rows(res);

	if (0 == row) {
		return row;
	}

	file_grid_buffer        *p_grid_buf = (file_grid_buffer *)args;
	file_grid_info          *ptr_info = p_grid_buf->p_buf;
	int64_t                 back_row = 0;
	MYSQL_ROW               res_row;

	while ((res_row = mysql_fetch_row(res))) {
		back_row++;
		uint32_t        id = atol(res_row[0]);
		char            *p_grid_id = res_row[1];

		if (!res_row[2]) {
			// x_printf(E,"grid line id:%d, PMRID is null", id);
			continue;
		}

		int32_t         num = atoi(res_row[2]);
		uint32_t        lon_num = 0;
		uint32_t        lat_num = 0;

		if (0 != convert_grid_id(p_grid_id, &lon_num, &lat_num)) {
			x_printf(E, "load grid error:%s\n", p_grid_id);
			continue;
		}

		/*取得经纬度范围*/
		if ((p_grid_buf->max_lon == 0) || (p_grid_buf->max_lon < lon_num)) {
			p_grid_buf->max_lon = lon_num;
		}

		if ((p_grid_buf->max_lat == 0) || (p_grid_buf->max_lat < lat_num)) {
			p_grid_buf->max_lat = lat_num;
		}

		if ((p_grid_buf->min_lon == 0) || (p_grid_buf->min_lon > lon_num)) {
			p_grid_buf->min_lon = lon_num;
		}

		if ((p_grid_buf->min_lat == 0) || (p_grid_buf->min_lat > lat_num)) {
			p_grid_buf->min_lat = lat_num;
		}

		int64_t offset = id - p_grid_buf->start_id;

		assert(offset < p_grid_buf->size);

		(ptr_info + offset)->id = id;
		(ptr_info + offset)->grid_lon = lon_num;
		(ptr_info + offset)->grid_lat = lat_num;
		(ptr_info + offset)->num = num;
	}

	return back_row;
}

/*生成grid count文件*/
static int32_t map_grid_gen_count_file(char *grid_count_file, map_grid_load_cfg *p_cfg)
{
	if (!grid_count_file || !p_cfg) {
		return ERR_PMR_PARAMETER;
	}

	FILE *p_file = fopen(grid_count_file, "wb");
	assert(p_file);

	grid_file_header header = {};
	memset(&header, 0, sizeof(header));
	header.min_id = 1;
	header.max_id = p_cfg->max_gridcount_id;

	// 写入文件头
	uint32_t size = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size == 1);

	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, p_cfg->host, p_cfg->port, p_cfg->database, p_cfg->username, p_cfg->passwd)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

	char                    *sql_format = "select ID, GridID, lineCount from %s.%s where id > %ld and id <= %ld";
	char                    sql[1024];
	unsigned int            start_id = 0;
	unsigned int            end_id = p_cfg->load_once;
	uint32_t                count = 0;
	file_grid_buffer        grid_buf;
	grid_buf.size = p_cfg->load_once;
	grid_buf.min_lon = 0;
	grid_buf.max_lon = 0;
	grid_buf.max_lat = 0;
	grid_buf.min_lat = 0;
	grid_buf.p_buf = (file_grid_info *)calloc(p_cfg->load_once, sizeof(file_grid_info));

	while (start_id <= p_cfg->max_gridcount_id) {
		memset(grid_buf.p_buf, 0, p_cfg->load_once * sizeof(file_grid_info));
		grid_buf.start_id = start_id + 1;

		sprintf(sql, sql_format, p_cfg->database, p_cfg->count_table, start_id, end_id);
		assert(0 < mysqlconn_cmd(&conn, sql, fetch_map_grid_cb, (void *)&grid_buf));

		// 将查询结果写到文件中
		uint32_t ret = fwrite((void *)grid_buf.p_buf, sizeof(file_grid_info), grid_buf.size, p_file);
		assert(ret == grid_buf.size);
		count += ret;
		x_printf(D, "gen map grid count, num:%d\n", count);

		start_id = end_id;
		end_id = end_id + p_cfg->load_once;
	}

	mysqlconn_disconnect(&conn);

	header.lon_begin = grid_buf.min_lon;
	header.lat_begin = grid_buf.min_lat;
	header.lon_size = (grid_buf.max_lon - header.lon_begin) / 5 + 1;
	header.lat_size = (grid_buf.max_lat - header.lat_begin) / 5 + 1;
	x_printf(D, "min_lon:%d,min_lat:%d,max_lon:%d,max:lat:%d", header.lon_begin, header.lat_begin, grid_buf.max_lon, grid_buf.max_lat);
	assert(fseek(p_file, 0, SEEK_SET) == 0);
	uint32_t size1 = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size1 == 1);

	fclose(p_file);

	return 0;
}

/*生成grid line文件*/
static int32_t map_grid_gen_line_file(char *grid_line_file, map_grid_load_cfg *p_cfg)
{
	if (!grid_line_file || !p_cfg) {
		return ERR_PMR_PARAMETER;
	}

	FILE *p_file = fopen(grid_line_file, "wb");
	assert(p_file);

	grid_file_header header = {};
	memset(&header, 0, sizeof(header));
	header.min_id = 1;
	header.max_id = p_cfg->max_gridline_id;

	// 写入文件头
	uint32_t size = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size == 1);

	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, p_cfg->host, p_cfg->port, p_cfg->database, p_cfg->username, p_cfg->passwd)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

	char                    *sql_format = "select ID, GridID, PMRID from %s.%s where id > %ld and id <= %ld";
	char                    sql[1024];
	unsigned int            start_id = 0;
	unsigned int            end_id = p_cfg->load_once;
	uint32_t                count = 0;
	file_grid_buffer        grid_buf;
	grid_buf.size = p_cfg->load_once;
	grid_buf.min_lon = 0;
	grid_buf.max_lon = 0;
	grid_buf.max_lat = 0;
	grid_buf.min_lat = 0;
	grid_buf.p_buf = (file_grid_info *)calloc(p_cfg->load_once, sizeof(file_grid_info));

	while (start_id <= p_cfg->max_gridline_id) {
		memset(grid_buf.p_buf, 0, p_cfg->load_once * sizeof(file_grid_info));
		grid_buf.start_id = start_id + 1;

		sprintf(sql, sql_format, p_cfg->database, p_cfg->line_table, start_id, end_id);
		assert(0 < mysqlconn_cmd(&conn, sql, fetch_map_grid_cb, (void *)&grid_buf));

		// 将查询结果写到文件中
		uint32_t ret = fwrite((void *)grid_buf.p_buf, sizeof(file_grid_info), grid_buf.size, p_file);
		assert(ret == grid_buf.size);
		count += ret;
		x_printf(D, "gen map grid line, num:%d\n", count);

		start_id = end_id;
		end_id = end_id + p_cfg->load_once;
	}

	mysqlconn_disconnect(&conn);

	header.lon_begin = grid_buf.min_lon;
	header.lat_begin = grid_buf.min_lat;
	header.lon_size = (grid_buf.max_lon - header.lon_begin) / 5 + 1;
	header.lat_size = (grid_buf.max_lat - header.lat_begin) / 5 + 1;

	x_printf(D, "min_lon:%d,min_lat:%d,max_lon:%d,max:lat:%d", header.lon_begin, header.lat_begin, grid_buf.max_lon, grid_buf.max_lat);

	assert(fseek(p_file, 0, SEEK_SET) == 0);
	uint32_t size1 = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size1 == 1);

	fclose(p_file);

	return 0;
}

/*生成网格文件*/
int32_t map_grid_gen_file(char *grid_count_file, char *grid_line_file, map_grid_load_cfg *p_cfg)
{
	if ((map_grid_gen_line_file(grid_line_file, p_cfg) != 0) ||
		(map_grid_gen_count_file(grid_count_file, p_cfg) != 0)) {
		return -1;
	} else {
		return 0;
	}
}

int32_t map_grid_manage_destory(map_grid_manager *p_manage)
{
	// TODO
	return 0;
}

