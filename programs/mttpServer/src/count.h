/*
 * Author       : chenzutao
 * Date         : 2015-11-20
 * Function     : count gps data, include gps numbers and stream size
 */

#ifndef __COUNT_H__
#define __COUNT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql.h>

#include "utils.h"
#include "libkv.h"
#include "rr_cfg.h"
#include "mttp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef DAT_ERR
#define DAT_ERR         GV_ERR
#define DAT_OK          GV_OK
#define DAT_BUF_SIZE    (1 << 7)
#define DAT_SQL_SIZE    (1 << 11)
#define CITY_SIZE       (1 << 9)
// #define DAT_TIME_INTERVAL 600

extern struct rr_cfg_file g_rr_cfg_file;
typedef struct data_count_s
{
	unsigned int    data_size;
	unsigned int    data_cnt;
	unsigned int    city_code;
	char            IMEI[32];
} data_count_t;

int city_data_count(kv_handler_t *handler, data_count_t *data);

int data_dump(kv_handler_t *handler, sql_conf_t *sql_conf);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COUNT_H__ */

