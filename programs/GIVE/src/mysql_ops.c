/**
 * Author       : chenzutao
 * Date         : 2015-10-26
 * Function     : mysql operation in C language
 **/

#include "mysql_ops.h"

int mysql_cmd(sql_conf_t *sql_conf, mysql_res_t **result)
{
	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "could not initialize MYSQL library.\n");
		return -1;
	}

	MYSQL           conn;
	MYSQL_RES       *res_ptr;
	MYSQL_ROW       res_row;

	int64_t row;
	long    cnt = 0;

	mysql_init(&conn);

	if (!mysql_real_connect(&conn,
		(sql_conf->host) ? sql_conf->host : "localhost",
		sql_conf->username,
		sql_conf->password,
		sql_conf->database,
		sql_conf->port,
		NULL, 0
		)) {
		printf("[** FAILED **] Failed to connect to [%s:%d]\n", sql_conf->host ? sql_conf->host : "localhost", sql_conf->port);

		if (mysql_errno(&conn)) {
			printf("connection error:%d -> %s .\n", mysql_errno(&conn), mysql_error(&conn));
		}

		mysql_close(&conn);
		mysql_library_end();
		return -1;
	}

	printf("SQL:%s\n", sql_conf->sql);

	if (mysql_query(&conn, sql_conf->sql)) {
		printf("[** FAILED **] query error:%s\n", mysql_error(&conn));
		mysql_close(&conn);
		mysql_library_end();
		return -1;
	}

	res_ptr = mysql_store_result(&conn);

	if (!res_ptr) {
		printf("[** FAILED **] result error:%s\n", mysql_error(&conn));
		mysql_free_result(res_ptr);
		mysql_close(&conn);
		mysql_library_end();
		return -1;
	}

	row = mysql_num_rows(res_ptr);
	printf("result rows:%ld\n", row);

	*result = (mysql_res_t *)calloc(sizeof(mysql_res_t), row);

	while ((res_row = mysql_fetch_row(res_ptr))) {
		(*result)[cnt].IMEI = atoll(res_row[1]);
		(*result)[cnt].type = atoi(res_row[2]);
		(*result)[cnt].status = atoi(res_row[3]);
		cnt++;
	}

	if (cnt != row) {
		printf("fetched rows:%ld, total rows:%ld\n", cnt, row);
	}

	mysql_free_result(res_ptr);
	mysql_close(&conn);
	mysql_library_end();

	return row;
}

#if 0
int main()
{
	sql_conf_t sql_conf = {
		.host           = "192.168.1.17",
		.username       = "dataTest",
		.password       = "DT456",
		.database       = "dataTest",
		.port           = 3306,
		.sql            = "select * from daoke_imei limit 10"
	};

	kv_handler_t *handler = kv_create(NULL);

	mysql_res_t     *result = NULL;
	int64_t         row = mysql_cmd(&sql_conf, &result);
	int             i;

	for (i = 0; i < row; i++) {
		printf("IMEI:%lld, type:%d, status:%d\n", result[i].IMEI, result[i].type, result[i].status);

		if (type_add(handler, result[i].IMEI, result[i].type)) {
			printf("[%d] failed .\n", i);
		}

		if (status_add(handler, result[i].IMEI, result[i].status)) {
			printf("[%d] failed .\n", i);
		}
	}

	free(result);
	kv_destroy(handler);

	return 0;
}
#endif	/* if 0 */

