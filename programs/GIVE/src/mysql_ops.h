/**
 * Author       : chenzutao
 * Date         : 2015-10-26
 * Function     : mysql operation in C language
 **/

#ifndef __MYSQL_OPS_H__
#define __MYSQL_OPS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>

#include "libkv.h"
#include "filter.h"

#include "utils.h"
#include "count.h"

/*typedef struct sql_conf{
 *        char *host;
 *        char *username;
 *        char *password;
 *        char *database;
 *        char *sql;
 *        unsigned int port;
 *   }sql_conf_t;*/

typedef struct mysql_res
{
	int             type;
	int             status;
	long long       IMEI;
} mysql_res_t;

int mysql_cmd(sql_conf_t *sql_conf, mysql_res_t **result);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __MYSQL_OPS_H__ */

