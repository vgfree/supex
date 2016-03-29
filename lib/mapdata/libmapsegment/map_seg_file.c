/*
 * 版权声明：暂无
 * 文件名称：map_seg_file.c
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
#include "map_seg_file.h"
#include "libmini.h"

static int64_t load_seg_info(MYSQL_RES *res, void *args)
{
	if (!res || !args) {
		return -1;
	}

	int64_t row = mysql_num_rows(res);

	if (0 == row) {
		return row;
	}

	sgid_file_header *p_head = (sgid_file_header *)args;

	if (p_head->load_once < row) {
		return row;
	}

	file_seg_info   *p_seg_buf = p_head->p_seg_buf;
	int64_t         back_row = 0;
	MYSQL_ROW       res_row;

	while ((res_row = mysql_fetch_row(res))) {
		file_seg_info *temp = p_seg_buf + back_row;

		if (res_row[0]) {
			temp->rrid = atoi(res_row[0]);
		}

		if (res_row[1]) {
			temp->sgid_id = atoi(res_row[1]);
		}

		if (res_row[2]) {
			temp->sgid = atoi(res_row[2]);
		}

		if (res_row[3] && (strlen(res_row[3]) > 2)) {
			char *token = strtok(res_row[3], ",");
			temp->next_rrid = atoi(token);
			token = strtok(NULL, ",");
			temp->next_sgid = atoi(token);
		}

		if (res_row[4]) {
			temp->name_id = atoi(res_row[4]);
		}

		if (res_row[5] && (strlen(res_row[5]) > 1)) {
			uint16_t long_name = strlen(res_row[5]) + 1;
			strncpy(temp->sg_name, res_row[5], long_name);
		}

		if (res_row[6]) {
			temp->start_name = atoi(res_row[6]);
		}

		if (res_row[7]) {
			temp->end_name = atoi(res_row[7]);
		}

		if (res_row[8]) {
			temp->start_lon = strtod(res_row[8], NULL);
		}

		if (res_row[9]) {
			temp->start_lat = strtod(res_row[9], NULL);
		}

		if (res_row[10]) {
			temp->end_lon = strtod(res_row[10], NULL);
		}

		if (res_row[11]) {
			temp->end_lat = strtod(res_row[11], NULL);
		}

		if (res_row[12]) {
			temp->start_grade = atoi(res_row[12]);
		}

		if (res_row[13]) {
			temp->end_grade = atoi(res_row[13]);
		}

		if (res_row[14]) {
			temp->length = atoi(res_row[14]);
		}

		if (res_row[15]) {
			temp->sgid_rt = atoi(res_row[15]);
		}

		if (res_row[16]) {
			temp->sgid_count = atoi(res_row[16]);
		}

                if (res_row[17]) {
                        temp->countyCode = atoi(res_row[17]);
                }

		back_row++;
	}

	return back_row;
}

#define SQL_TABLE_PRE   "select   RRID, id, SGID, next_id, name_id, name, SGSC, SGEC, SGSL, SGSB, SGEL, SGEB, SGFER, SGTCR, sg_len, sg_rt, sg_count, countyCode  from "
#define SQL_TABLE_NEXT  " where iidd  between %d and %d"

int map_seg_file(char *sgid_file, map_seg_cfg *p_seg_cfg)
{
	if (!p_seg_cfg || !sgid_file) {
		return -1;
	}

	if ((p_seg_cfg->max_index <= 0) || (p_seg_cfg->min_index <= 0) || (p_seg_cfg->max_index < p_seg_cfg->min_index)) {
		return -1;
	}

	sgid_file_header *header = (sgid_file_header *)calloc(1, sizeof(sgid_file_header));

	if (!header) {
		return -1;
	}

	header->max_index = p_seg_cfg->max_index;
	header->min_index = p_seg_cfg->min_index;
	header->load_once = p_seg_cfg->load_once;

	header->index_long = p_seg_cfg->index_long;
	header->rrid_buf_long = p_seg_cfg->rrid_buf_long;
	header->name_index_long = p_seg_cfg->name_index_long;
	header->name_buf_long = p_seg_cfg->name_buf_long;

	FILE *p_file = fopen(sgid_file, "wb");
	assert(p_file);
	uint32_t size = fwrite((void *)header, sizeof(sgid_file_header), 1, p_file);
	assert(size == 1);
	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, p_seg_cfg->host, p_seg_cfg->port, p_seg_cfg->database, p_seg_cfg->username, p_seg_cfg->passwd)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

	char montage_sql[521] = {};
	memset(montage_sql, 0, 521);
	strcat(montage_sql, SQL_TABLE_PRE);
	strcat(montage_sql, p_seg_cfg->table);
	strcat(montage_sql, SQL_TABLE_NEXT);
	uint64_t        pre_index = header->min_index;
	uint64_t        behind_index = header->min_index + header->load_once - 1;

	if (pre_index > behind_index) {
		return -1;
	}

	uint64_t        start_index = 0;
	char            sql_buf[521] = {};
	uint64_t        count = 0;

	while (start_index <= header->max_index) {
		memset(sql_buf, 0, 521);
		sprintf(sql_buf, montage_sql, pre_index, behind_index);
		printf(" %s\n", sql_buf);
		header->p_seg_buf = (file_seg_info *)calloc(header->load_once, sizeof(file_seg_info));

		if (!header->p_seg_buf) {
			x_printf(E, "ptr_sgid calloc menmory faile\n");
		}

		uint64_t row_back = mysqlconn_cmd(&conn, sql_buf, load_seg_info, (void *)header);

		if (row_back <= 0) {
			free(header->p_seg_buf);
			header->p_seg_buf = NULL;
			break;
		}

		count = count + row_back;
		fwrite((void *)header->p_seg_buf, sizeof(file_seg_info), header->load_once, p_file);
		free(header->p_seg_buf);
		header->p_seg_buf = NULL;

		if (pre_index == behind_index) {
			break;
		}

		pre_index = behind_index + 1;
		start_index = pre_index;
		behind_index = pre_index + header->load_once - 1;

		if (behind_index > header->max_index) {
			behind_index = header->max_index;
		}
	}

        x_printf(D, "count= %-10ld\n", count);
	free(header);
	header = NULL;
	return 0;
}

