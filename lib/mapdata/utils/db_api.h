/**/
#pragma once

#include <mysql.h>
#include <stdint.h>

#define MYSQL_CONN_STR_MAX 32

typedef struct mysqlconn
{
	uint16_t        port;
	char            host[MYSQL_CONN_STR_MAX];
	char            username[MYSQL_CONN_STR_MAX];
	char            passwd[MYSQL_CONN_STR_MAX];
	char            database[MYSQL_CONN_STR_MAX];
	MYSQL           conn;
} mysqlconn;

typedef int64_t (*SQLFUN_CALLBACK)(MYSQL_RES *res, void *args);

int32_t mysqlconn_init(mysqlconn *p_conn, char *p_host, uint16_t port, char *p_dbname, char *p_user, char *p_pwd);

int32_t mysqlconn_connect(mysqlconn *p_conn);

int64_t mysqlconn_cmd(mysqlconn *p_conn, char *p_sql, SQLFUN_CALLBACK excb, void *args);

void mysqlconn_disconnect(mysqlconn *p_conn);

