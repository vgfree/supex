#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "db_api.h"
#include "libmini.h"

static int64_t fetch_line_info(MYSQL_RES *res, void *args)
{
	return 0;
}

int main()
{
	char            *host = "192.168.1.10";
	char            *username = "observer";
	char            *password = "abc123";
	char            *database = "roadMap3";
	char            *sql = "select * from mysql.user";
	uint16_t        port = 3306;

	mysqlconn conn;

	if (0 != mysqlconn_init(&conn, host, port, database, username, password)) {
		return -1;
	}

	if (0 != mysqlconn_connect(&conn)) {
		return -1;
	}

	int64_t size = mysqlconn_cmd(&conn, sql, fetch_line_info, NULL);

	x_printf(D, "result size:%ld\n", size);

	mysqlconn_disconnect(&conn);

	return 0;
}

