#include "topo_file.h"
#include "map_errno.h"
#include "db_api.h"
#include "topo_info.h"
#include "libmini.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct topo_file_header_t {
        uint32_t max_id;
        uint32_t max_rr_id;
        uint16_t load_once;
}topo_file_header_t;

typedef struct topo_file_line_buff_t{
        topo_file_line_t *p_line_buff;
        uint32_t max_rr_id;
}topo_file_line_buff_t;


static int64_t fetch_topo_line_cb(MYSQL_RES *res, void *args)
{
        if (!res || !args) {
                return -1;
        }

        int64_t row = mysql_num_rows(res);

        if (0 == row) {
                return row;
        }

        topo_file_line_buff_t *p_file_buff = (topo_file_line_buff_t *)args;
        topo_file_line_t *p_line_buf = p_file_buff->p_line_buff;

        int64_t         back_row = 0;
        MYSQL_ROW       res_row;

        while ((res_row = mysql_fetch_row(res))) {
                topo_file_line_t *temp = p_line_buf + back_row;
                temp->conn_type = TOPO_CONN_TYPE_DIRECT;

                if (res_row[0]) {
                        temp->id = atoi(res_row[0]);
                }

                if (res_row[1]) {
                        temp->rrid1 = atoi(res_row[1]);
                }

                if (res_row[2]) {
                        temp->sgid1 = atoi(res_row[2]);
                }

                if (res_row[3]) {
                        temp->count1 = atoi(res_row[3]);
                }

                if (res_row[4]) {
                        temp->rrid2 = atoi(res_row[4]);
                }

                if (res_row[5]) {
                        temp->sgid2 = atoi(res_row[5]);
                }

                if (res_row[6]) {
                        temp->count2 = atoi(res_row[6]);
                }

                if (res_row[7]) {
                        temp->conn_count = atoi(res_row[7]);
                }

                if (res_row[8]) {
                        temp->SL = strtod(res_row[8], NULL);
                        temp->conn_type = TOPO_CONN_TYPE_BYROAD;
                }

                if (res_row[9]) {
                        temp->SB = strtod(res_row[9], NULL);
                }

                if (res_row[10]) {
                        temp->EL = strtod(res_row[10], NULL);
                }

                if (res_row[11]) {
                        temp->EB = strtod(res_row[11], NULL);
                }

                back_row++;

                if(temp->rrid1 > p_file_buff->max_rr_id)
                        p_file_buff->max_rr_id = temp->rrid1;
        }

        return back_row;
}

int topo_file_gen(topo_cfg_mysql_t *p_cfg)
{
        if (!p_cfg || !p_cfg->map_topo_file) {
                return ERR_PMR_PARAMETER;
        }

        char *file_name = p_cfg->map_topo_file;

        FILE *p_file = fopen(file_name, "wb");
        assert(p_file);

        topo_file_header_t header = {};
        header.max_id = p_cfg->max_id;
        header.max_rr_id = 0;
        header.load_once = p_cfg->load_once;


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

        char                    *sql_format = "select id, RR1, SG1, count1, RR2,SG2,count2,con_count, SL,SB,EL,EB  from %s.%s where id > %d and id <= %d";
        char                    sql[1024];
        unsigned int            start_id = 0;
        unsigned int            end_id = p_cfg->load_once;
        long                    count = 0;
        topo_file_line_buff_t line_buff;
        line_buff.p_line_buff = (topo_file_line_t *)malloc(p_cfg->load_once * sizeof(topo_file_line_t));

        while (start_id <= p_cfg->max_id) {
                memset(line_buff.p_line_buff, 0, p_cfg->load_once * sizeof(topo_file_line_t));
                sprintf(sql, sql_format, p_cfg->database, p_cfg->table, start_id, end_id);
                printf("%s\n", sql);
                assert(0 <= mysqlconn_cmd(&conn, sql, fetch_topo_line_cb, (void *)&line_buff));

                // 将查询结果写到文件中
                uint32_t ret = fwrite((void *)line_buff.p_line_buff,  sizeof(topo_file_line_t), p_cfg->load_once, p_file);
                assert(ret == p_cfg->load_once);
                count += ret;

                start_id = end_id;
                end_id = end_id + p_cfg->load_once;
        }

        header.max_rr_id = line_buff.max_rr_id;
        assert(fseek(p_file, 0, SEEK_SET) == 0);
        uint32_t size1 = fwrite((void *)&header, sizeof(header), 1, p_file);
        assert(size1 == 1);

        mysqlconn_disconnect(&conn);
        fclose(p_file);

        return 0;
}

int topo_file_load(map_topo_manager_t *p_manage)
{
        uint32_t        count = 0;
        FILE            *p_file = fopen(p_manage->file_cfg.topo_file, "rb");	// 二进制模式打开

        assert(p_file);

        // 读文件头
        topo_file_header_t        header = {};
        uint32_t                size = fread((void *)&header, sizeof(topo_file_header_t), 1, p_file);
        assert(size == 1);

        if (header.max_rr_id <= 0) {
                x_printf(E, "load map line file error, max_rr_id:%d\n", header.max_rr_id);
                return -1;
        }

        assert(map_topo_manager_init_buff(p_manage, header.load_once, header.max_rr_id) == 0);

        // 分配预读buffer
        uint16_t load_once = header.load_once;
        uint32_t        index_count = header.max_rr_id / load_once;
        int             last_count = header.max_rr_id % load_once;
        index_count = (last_count == 0) ? index_count : index_count + 1;
        topo_file_line_t *p_buf = (topo_file_line_t *)malloc(load_once * sizeof(topo_file_line_t));

        // 分段与读文件到内存中
        int index;

        for (index = 0; index < index_count; index++) {
                int step_len = load_once;

                if ((index == (index_count - 1)) && (last_count != 0)) {/*最后一次，取剩余数量*/
                        step_len = last_count;
                }

                uint32_t ret = fread((void *)p_buf, sizeof(topo_file_line_t), step_len, p_file);

                if (ret <= 0) {
                        continue;
                }

                assert(ret == step_len);

                /*遍历每一个元素，分配存储lineID的内存*/
                int i;

                for (i = 0; i < ret; i++) {
                        topo_file_line_t *p_line = p_buf + i;
                        if(p_line->rrid1 == 0 || p_line->sgid1 == 0)
                                continue;

                        map_topo_manager_add_line(p_manage, p_line);
                }

                count += ret;
                x_printf(I, "load map gird count, num:%d\n", count);
        }

        fclose(p_file);
        return count;
}
