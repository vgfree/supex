#include "map_poi_file.h"
#include "map_utils.h"
#include "db_api.h"
#include "libmini.h"
#include "map_poi.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct mappoi_file_header
{
	int             load_once;
	unsigned int    max_id;
	unsigned int    max_rr_id;
	int             buf_count;
} mappoi_file_header;

static int mappoi_gen_file_poi(mysqlconn *p_conn, mappoi_cfg_mysql *p_cfg);

static int mappoi_gen_file_sg(mysqlconn *p_conn, mappoi_cfg_mysql *p_cfg);

static int mappoi_load_poi(mappoi_manager *p_manager);

static int mappoi_load_sg(mappoi_manager *p_manager);

int mappoi_gen_file(mappoi_cfg_mysql *p_cfg)
{
	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, p_cfg->host, p_cfg->port, p_cfg->database, p_cfg->username, p_cfg->passwd)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

	if ((mappoi_gen_file_poi(&conn, p_cfg) < 0) ||
		(mappoi_gen_file_sg(&conn, p_cfg) < 0)) {
		mysqlconn_disconnect(&conn);
		return -1;
	}

	return 0;
}

int mappoi_load_file(mappoi_manager *p_manager)
{
	if (!p_manager) {
		return -1;
	}

	if ((strlen(p_manager->file_name_poi) <= 0) || (strlen(p_manager->file_name_sg) <= 0)) {
		return -1;
	}

	if ((mappoi_load_sg(p_manager) != 0) ||
		(mappoi_load_poi(p_manager) != 0)) {
		return -1;
	}

	return 0;
}

static int mappoi_load_sg(mappoi_manager *p_manager)
{
	uint32_t        count = 0;
	FILE            *p_file = fopen(p_manager->file_name_sg, "rb");	// 二进制模式打开

	assert(p_file);

	// 读文件头
	mappoi_file_header      header = {};
	uint32_t                size = fread((void *)&header, sizeof(mappoi_file_header), 1, p_file);
	assert(size == 1);

	if ((header.max_id <= 0) || (header.load_once <= 0) || (header.max_rr_id <= 0)) {
		x_printf(E, "mappoi_load_sg error, max_id:%d\n", header.max_id);
		return -1;
	}

	// 分配索引空间
	uint32_t rr_index_count = header.max_rr_id / header.load_once;
	rr_index_count = (header.max_rr_id % header.load_once == 0) ? rr_index_count : rr_index_count + 1;
	p_manager->load_once = header.load_once;
	p_manager->rr_index_count = rr_index_count;
	p_manager->max_sg_id = header.max_id;

	int index;

	p_manager->rr_buf = (mappoi_rr **)calloc(rr_index_count, sizeof(mappoi_rr *));
	assert(p_manager->rr_buf);

	for (index = 0; index < rr_index_count; index++) {
		p_manager->rr_buf[index] = (mappoi_rr *)calloc(p_manager->load_once, sizeof(mappoi_rr));
	}

	mappoi_sg *sg_load_buf = (mappoi_sg *)calloc(p_manager->load_once, sizeof(mappoi_sg));
	assert(sg_load_buf);
	// 加载line具体信息

	for (index = 0; index < header.buf_count; index++) {
		count++;
		int size = fread((void *)sg_load_buf, sizeof(mappoi_sg), p_manager->load_once, p_file);
		assert(size == p_manager->load_once);

		int i;

		for (i = 0; i < size; i++) {
			mappoi_sg *p_sg = sg_load_buf + i;

			if (p_sg->rr_id == 0) {
				continue;
			}

			unsigned int    rr_id = p_sg->rr_id;
			int             rr_idx = rr_id / p_manager->load_once;
			int             rr_count = rr_id % p_manager->load_once;

			mappoi_rr *p_rr = p_manager->rr_buf[rr_idx] + rr_count;
			assert(p_rr);

			if (p_rr->rr_id == 0) {
				p_rr->rr_id = rr_id;
			} else if (p_rr->rr_id != rr_id) {
				x_printf(E, "load rrid error");
				return -1;
			}

			if ((p_rr->sg.sg_count == 0) && (p_rr->sg.sg_buf == NULL)) {
				p_rr->sg.sg_count = p_sg->sg_count;
				p_rr->sg.sg_buf = (mappoi_sg *)calloc(p_sg->sg_count, sizeof(mappoi_sg));
			}

			assert(p_sg->sg_id <= p_rr->sg.sg_count);
			// x_printf(D, "sg_count:%d, sg_id:%d", p_sg->sg_count, p_sg->sg_id);
			(p_rr->sg.sg_buf)[p_sg->sg_id - 1] = *p_sg;
		}

		// x_printf(D, "load mappoi_sg count, num:%d", count * p_manager->load_once);
	}

	fclose(p_file);

	return 0;
}

static int mappoi_load_poi(mappoi_manager *p_manager)
{
	uint32_t        count = 0;
	FILE            *p_file = fopen(p_manager->file_name_poi, "rb");// 二进制模式打开

	assert(p_file);

	// 读文件头
	mappoi_file_header      header = {};
	uint32_t                size = fread((void *)&header, sizeof(mappoi_file_header), 1, p_file);
	assert(size == 1);

	if ((header.max_id <= 0) || (header.load_once <= 0) || (header.buf_count <= 0)) {
		x_printf(E, "mappoi_load_sg error, max_id:%d\n", header.max_id);
		return -1;
	}

	mappoi_poi *poi_buf = (mappoi_poi *)calloc(header.load_once, sizeof(mappoi_poi));

	int index;

	for (index = 0; index < header.buf_count; index++) {
		count++;
		memset(poi_buf, 0, header.load_once * sizeof(mappoi_poi));
		int size = fread((void *)poi_buf, sizeof(mappoi_poi), p_manager->load_once, p_file);
		assert(size == p_manager->load_once);

		int poi_idx;

		for (poi_idx = 0; poi_idx < size; poi_idx++) {
			mappoi_poi *p_poi = poi_buf + poi_idx;

			if (p_poi->rr_id == 0) {
				continue;
			}

			unsigned int    rr_id = p_poi->rr_id;
			int             rr_idx = rr_id / p_manager->load_once;
			int             rr_count = rr_id % p_manager->load_once;

			mappoi_rr *p_rr = p_manager->rr_buf[rr_idx] + rr_count;
			assert(p_rr);

			if (p_rr->rr_id != rr_id) {
				x_printf(E, "load rrid error");
				return -1;
			}

			mappoi_poi_buf poi_buf = p_rr->sg.sg_buf[p_poi->sg_id - 1].poi_buf;

			if ((poi_buf.poi_count == 0) || (poi_buf.poi == NULL)) {
				poi_buf.poi_count = p_poi->poi_count;
				poi_buf.poi = (mappoi_poi *)calloc(p_poi->poi_count, sizeof(mappoi_poi));
				poi_buf.poi_idx = 0;
			}

			assert(poi_buf.poi_idx < poi_buf.poi_count);
			poi_buf.poi[poi_buf.poi_idx] = *p_poi;
			poi_buf.poi_idx++;

			p_rr->sg.sg_buf[p_poi->sg_id - 1].poi_buf.poi = poi_buf.poi;
			p_rr->sg.sg_buf[p_poi->sg_id - 1].poi_buf.poi_idx = poi_buf.poi_idx;
			p_rr->sg.sg_buf[p_poi->sg_id - 1].poi_buf.poi_count = poi_buf.poi_count;
		}

		// x_printf(D, "load mappoi_poi count, num:%d", count * p_manager->load_once);
	}

	fclose(p_file);

	return 0;
}

// 处理数据库查询结果回调函数
static int64_t fetch_mappoi_poi_cb(MYSQL_RES *res, void *args)
{
	if (!res || !args) {
		return -1;
	}

	int64_t row = mysql_num_rows(res);

	if (0 == row) {
		return row;
	}

	mappoi_poi_buf  *poi_buf = (mappoi_poi_buf *)args;
	int64_t         back_row = 0;
	MYSQL_ROW       res_row;

	while ((res_row = mysql_fetch_row(res))) {
		back_row++;
		mappoi_poi *poi = (poi_buf->poi) + (back_row - 1);

		poi->poi_id = atol(res_row[0]);
		poi->point_x = strtod(res_row[1], NULL);
		poi->point_y = strtod(res_row[2], NULL);
		poi->poi_type = atol(res_row[3]);
		poi->county_code = atol(res_row[4]);
		poi->city_code = atol(res_row[5]);
		poi->ang = atol(res_row[6]);
		poi->sg_id = atol(res_row[7]);
		poi->rr_id = atol(res_row[8]);
		poi->poi_count = atol(res_row[9]);

		assert(back_row <= poi_buf->poi_count);
	}

	return back_row;
}

static int mappoi_gen_file_poi(mysqlconn *p_conn, mappoi_cfg_mysql *p_cfg)
{
	FILE *p_file = fopen(p_cfg->file_name_poi, "wb");

	assert(p_file);

	char                    *sql_format = "SELECT POI_ID,POINT_X,POINT_Y,POI_TYPE,countyCode, cityCode,ang,SGID,RRID,count_poi FROM %s.%s where ID > %ld and ID <= %ld";
	char                    sql[1024];
	unsigned int            start_id = 0;
	unsigned int            end_id = p_cfg->load_once;
	mappoi_file_header      header;
	header.load_once = p_cfg->load_once;
	header.max_id = p_cfg->max_poi_id;

	uint32_t size = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size == 1);

	mappoi_poi      *poi_load_buf = (mappoi_poi *)calloc(p_cfg->load_once, sizeof(mappoi_poi));
	mappoi_poi_buf  poi_buf;
	poi_buf.poi = poi_load_buf;
	poi_buf.poi_count = p_cfg->load_once;
	int count = 0;

	while (start_id <= p_cfg->max_poi_id) {
		memset(poi_buf.poi, 0, poi_buf.poi_count * sizeof(mappoi_poi));

		sprintf(sql, sql_format, p_cfg->database, p_cfg->poi_table, start_id, end_id);
		assert(0 < mysqlconn_cmd(p_conn, sql, fetch_mappoi_poi_cb, (void *)&poi_buf));

		// 将查询结果写到文件中
		uint32_t ret = fwrite((void *)poi_buf.poi, sizeof(mappoi_poi), poi_buf.poi_count, p_file);
		assert(ret == p_cfg->load_once);
		count++;
		x_printf(D, "gen mappoi_poi count, num:%d", count * p_cfg->load_once);

		start_id = end_id;
		end_id = end_id + p_cfg->load_once;
	}

	assert(fseek(p_file, 0, SEEK_SET) == 0);
	header.buf_count = count;
	int size1 = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size1 == 1);

	fclose(p_file);

	return 0;
}

// 处理数据库查询结果回调函数
static int64_t fetch_mappoi_sg_cb(MYSQL_RES *res, void *args)
{
	if (!res || !args) {
		return -1;
	}

	int64_t row = mysql_num_rows(res);

	if (0 == row) {
		return row;
	}

	mappoi_sg_buf   *sg_buf = (mappoi_sg_buf *)args;
	int64_t         back_row = 0;
	MYSQL_ROW       res_row;
	unsigned int    max_rr_id = 0;
	int             load_once = sg_buf->sg_count;

	while ((res_row = mysql_fetch_row(res))) {
		back_row++;
		mappoi_sg *sg = (sg_buf->sg_buf) + (back_row - 1);

		sg->rr_id = atol(res_row[0]);
		sg->sg_id = atol(res_row[1]);

		if (res_row[2] && (strlen(res_row[2]) > 2)) {
			char *token = strtok(res_row[2], ",");
			sg->next_rr_id = atoi(token);
			token = strtok(NULL, ",");
			sg->next_sg_id = atoi(token);
		}

		sg->sg_count = atol(res_row[3]);

		if (sg->rr_id > max_rr_id) {
			max_rr_id = sg->rr_id;
		}

		assert(back_row <= load_once);
	}

	sg_buf->sg_count = max_rr_id;	// 临时存储最大rr_id传出

	return back_row;
}

static int mappoi_gen_file_sg(mysqlconn *p_conn, mappoi_cfg_mysql *p_cfg)
{
	FILE *p_file = fopen(p_cfg->file_name_sg, "wb");

	assert(p_file);

	char                    *sql_format = "SELECT RRID, SGID, next_id,sg_count FROM %s.%s where iidd > %ld and iidd <= %ld;";
	char                    sql[1024];
	unsigned int            start_id = 0;
	unsigned int            end_id = p_cfg->load_once;
	mappoi_file_header      header;
	header.load_once = p_cfg->load_once;
	header.max_id = p_cfg->max_sg_id;
	header.max_rr_id = 0;

	uint32_t size = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size == 1);

	mappoi_sg       *sg_load_buf = (mappoi_sg *)calloc(p_cfg->load_once, sizeof(mappoi_sg));
	mappoi_sg_buf   sg_buf;
	sg_buf.sg_buf = sg_load_buf;
	int     count = 0;
	int     max_rr_id = 0;

	while (start_id <= p_cfg->max_sg_id) {
		sg_buf.sg_count = p_cfg->load_once;
		memset(sg_buf.sg_buf, 0, sg_buf.sg_count * sizeof(mappoi_sg));

		sprintf(sql, sql_format, p_cfg->database, p_cfg->sg_table, start_id, end_id);
		assert(0 < mysqlconn_cmd(p_conn, sql, fetch_mappoi_sg_cb, (void *)&sg_buf));

		if (sg_buf.sg_count > max_rr_id) {
			max_rr_id = sg_buf.sg_count;	// 在回调函数中统计最大rrid，由sg_buf传出。
		}

		// 将查询结果写到文件中
		uint32_t ret = fwrite((void *)sg_buf.sg_buf, sizeof(mappoi_sg), p_cfg->load_once, p_file);
		assert(ret == p_cfg->load_once);
		count++;
		x_printf(D, "gen mappoi_sg count, num:%d", count * p_cfg->load_once);

		start_id = end_id;
		end_id = end_id + p_cfg->load_once;
	}

	assert(fseek(p_file, 0, SEEK_SET) == 0);
	header.buf_count = count;
	header.max_rr_id = max_rr_id;
	int size1 = fwrite((void *)&header, sizeof(header), 1, p_file);
	assert(size1 == 1);

	fclose(p_file);

	return 0;
}

