#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "db_api.h"
#include "map_errno.h"

int32_t mysqlconn_init(mysqlconn *p_conn, char *p_host, uint16_t port, char *p_database, char *p_user, char *p_pwd)
{
	if (!p_conn || !p_host || !p_database || !p_user || !p_pwd || (port <= 0)) {
		return ERR_PMR_PARAMETER;
	}

	strncpy(p_conn->host, p_host, MYSQL_CONN_STR_MAX - 1);
	p_conn->port = port;
	strncpy(p_conn->database, p_database, MYSQL_CONN_STR_MAX - 1);
	strncpy(p_conn->username, p_user, MYSQL_CONN_STR_MAX - 1);
	strncpy(p_conn->passwd, p_pwd, MYSQL_CONN_STR_MAX - 1);

	return 0;
}

int32_t mysqlconn_connect(mysqlconn *p_conn)
{
	mysql_init(&p_conn->conn);

	if (!mysql_real_connect(&p_conn->conn, p_conn->host, p_conn->username, p_conn->passwd, p_conn->database, p_conn->port, NULL, 0)) {
		fprintf(stderr, "Connection failed\n");

		if (mysql_errno(&p_conn->conn)) {
			printf("Connection error %d: %s\n", mysql_errno(&p_conn->conn), mysql_error(&p_conn->conn));
		}

		return ERR_MYSQL_CONNECT_FAILED;
	}

	if (mysql_set_character_set(&p_conn->conn, "utf8")) {
		fprintf(stderr, "mysql_set_character_set\n");
	}

	return 0;
}

int64_t mysqlconn_cmd(mysqlconn *p_conn, char *sql, SQLFUN_CALLBACK excb, void *args)
{
	if (mysql_real_query(&p_conn->conn, sql, (unsigned int)strlen(sql))) {
		printf("SELECT error:%s\n", mysql_error(&p_conn->conn));
		mysql_close(&p_conn->conn);
		return ERR_MYSQL_COMMAND_FAILED;
	}

	MYSQL_RES *res = mysql_store_result(&p_conn->conn);

	if (!res) {
		printf("mysql error:%s\n", mysql_error(&p_conn->conn));
		mysql_free_result(res);
		mysql_close(&p_conn->conn);
		return ERR_MYSQL_DISPOSE_FAILED;
	}

	int64_t row = mysql_num_rows(res);
	int64_t back_row = excb(res, args);

	if ((back_row != row) && mysql_errno(&p_conn->conn)) {
		printf("Retrive error:%s\n", mysql_error(&p_conn->conn));
	}

	mysql_free_result(res);

	return row;
}

void mysqlconn_disconnect(mysqlconn *p_conn)
{
	mysql_close(&p_conn->conn);
}

