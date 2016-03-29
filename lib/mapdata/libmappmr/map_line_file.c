/*
 * 版权声明：暂无
 * 文件名称：map_line_file.c
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/30
 * 文件描述：load data from file
 * 历史记录：
 */

#include "map_line_file.h"
#include "map_line.h"
#include "map_errno.h"
#include "db_api.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct line_file_header
{
	uint32_t        min_id;
	uint32_t        max_id;
} line_file_header;

typedef struct line_file_buffer
{
	map_line_info   *p_buf;
	uint32_t        size;
	uint32_t        start_id;
} line_file_buffer;

/* 功    能：从文件中加在line数据 */
int32_t map_line_load_file(char *file_name, map_line_manager *p_manage, uint32_t load_once)
{
	uint32_t        count = 0;
	FILE            *p_file = fopen(file_name, "rb");	// 二进制模式打开

	assert(p_file);

	// 读文件头
	line_file_header        header = {};
	uint32_t                size = fread((void *)&header, sizeof(line_file_header), 1, p_file);
	assert(size == 1);

	if ((header.max_id <= 0) || (header.min_id <= 0)) {
		x_printf(E, "load map line file error, min_id:%d, max_id:%d\n", header.min_id, header.max_id);
		return -1;
	}

	// 分配索引空间
	uint32_t index_count = header.max_id / load_once;
	index_count = (header.max_id % load_once == 0) ? index_count : index_count + 1;
	map_line_info **pp_lines = (map_line_info **)calloc(index_count, sizeof(map_line_info *));
	assert(pp_lines);

	// 加载line具体信息
	int index;

	for (index = 0; index < index_count; index++) {
		map_line_info *p_buf = (map_line_info *)calloc(load_once, sizeof(map_line_info));
		fread((void *)p_buf, sizeof(map_line_info), load_once, p_file);
		pp_lines[index] = p_buf;
                //x_printf(D, "load line info count:%d", index * load_once);
	}

	fclose(p_file);
	p_manage->ptr_arry = pp_lines;
	p_manage->min_line_id = header.min_id;
        p_manage->max_line_id = header.max_id;
	p_manage->destory_long = index_count;

	return count;
}

// 释放 manage和line的内存
int map_line_manager_destory(map_line_manager *p_manage)
{
	if (NULL == p_manage) {
		return 0;
	}

	int64_t i = 0;

	for (; i < p_manage->destory_long; i++) {
		if (p_manage->ptr_arry[i]) {
			free(p_manage->ptr_arry[i]);
			p_manage->ptr_arry[i] = NULL;
		}
	}

	free(p_manage->ptr_arry);
	p_manage->ptr_arry = NULL;
	return 0;
}

// 处理数据库查询结果回调函数
static int64_t fetch_line_cb(MYSQL_RES *res, void *args)
{
	if (!res || !args) {
		return -1;
	}

	int64_t row = mysql_num_rows(res);

	if (0 == row) {
		return row;
	}

	line_file_buffer        *p_line_buf = (line_file_buffer *)args;
	map_line_info           *ptr_info = p_line_buf->p_buf;
	int64_t                 back_row = 0;
	MYSQL_ROW               res_row;

	while ((res_row = mysql_fetch_row(res))) {
		back_row++;
		int64_t line_id_number = atol(res_row[0]);
		int64_t offset = line_id_number - p_line_buf->start_id;

		assert(offset < p_line_buf->size);

		(ptr_info + offset)->line_id = line_id_number;
                (ptr_info + offset)->start_lon = strtod(res_row[1], NULL);
                (ptr_info + offset)->start_lat = strtod(res_row[2], NULL);
                (ptr_info + offset)->end_lon = strtod(res_row[3], NULL);
                (ptr_info + offset)->end_lat = strtod(res_row[4], NULL);
                (ptr_info + offset)->dir = atoi(res_row[5]);
                (ptr_info + offset)->sgid = (unsigned int)atol(res_row[6]);
//                (ptr_info + offset)->tfid = (unsigned int)atol(res_row[7]);
                (ptr_info + offset)->tfid = 0;
                (ptr_info + offset)->rr_id = atol(res_row[8]);

		/*
		 *   if( res_row[11])
		 *   {
		 *        char *token = strtok( res_row[11], ",");
		 *        (ptr_info + offset)->next_rr_id = atoi(token);
		 *        token = strtok( NULL, ",");
		 *        (ptr_info + offset)->next_sgid = atoi(token);
		 *   }
		 */
	}

	return back_row;
}

/* 功    能：生成line数据，提供给map_line_load_file加载使用 */
int32_t map_line_gen_file(char *file_name, map_line_load_cfg *p_line_cfg)
{
	if (!file_name || !p_line_cfg) {
		return ERR_PMR_PARAMETER;
	}

	FILE *p_file = fopen(file_name, "wb");
	assert(p_file);

	line_file_header header = {};
	header.min_id = p_line_cfg->min_id;
	header.max_id = p_line_cfg->max_id;

	// 写入文件头
	uint32_t size = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size == 1);

	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, p_line_cfg->host, p_line_cfg->port, p_line_cfg->database, p_line_cfg->username, p_line_cfg->passwd)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

        char                    *sql_format = "select PMRID, SL, SB, EL, EB, ANG, SGID, TFID, RRID   from %s.%s where PMRID > %d and PMRID <= %d";
	char                    sql[1024];
	unsigned int            start_id = 0;
	unsigned int            end_id = p_line_cfg->load_once;
	long                    count = 0;
	line_file_buffer        line_buf;
	line_buf.size = p_line_cfg->load_once;
	line_buf.p_buf = (map_line_info *)calloc(p_line_cfg->load_once, sizeof(map_line_info));

	while (start_id <= p_line_cfg->max_id) {
		memset(line_buf.p_buf, 0, p_line_cfg->load_once * sizeof(map_line_info));
		line_buf.start_id = start_id + 1;

		sprintf(sql, sql_format, p_line_cfg->database, p_line_cfg->table, start_id, end_id);
		printf("%s\n", sql);
		assert(0 <= mysqlconn_cmd(&conn, sql, fetch_line_cb, (void *)&line_buf));

		// 将查询结果写到文件中
		uint32_t ret = fwrite((void *)line_buf.p_buf, sizeof(map_line_info), line_buf.size, p_file);
		assert(ret == line_buf.size);
		count += ret;

		start_id = end_id;
		end_id = end_id + p_line_cfg->load_once;
	}

	mysqlconn_disconnect(&conn);
	fclose(p_file);

	return 0;
}

