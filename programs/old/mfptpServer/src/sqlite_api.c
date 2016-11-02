#include "sqlite_api.h"
#include <stdio.h>

sqlite3 *g_db = NULL;

/*
 * 函数名:sqlite_init
 * 功能:创建数据
 * 参数:db_name数据库名
 */
int64_t sqlite_init(struct sql_info *info)
{
	if (SQLITE_OK == sqlite3_open(info->database, &g_db)) {
		return TRUE;
	}

	return FALSE;
}

/*
 * 函数名:sqlite_cmd
 * 功能：调用sqlite执行sql语句，对返回结果调用cb回调函数进行处理
 * 参数:sql 要执行的sql语句,cb 回调函数
 * 返回值:成功返回ＴＲＵＥ，失败返回ＦＡＬＳＥ
 */
int64_t sqlite_cmd(struct sql_info *info, SQLITE_CALLBACK cb)
{
	int64_t res = FALSE;

	fprintf(stderr, "FALSE= %ld\n", res);

	if ((NULL != g_db) && (NULL != info) && (NULL != info->sql)) {
		int temp_res = sqlite3_exec(g_db, info->sql, cb, NULL, NULL);

		if (SQLITE_OK == temp_res) {
			res = TRUE;
		}
	}

	return res;
}

/*
 * 函数名:sqlite_close
 * 功能:关闭sqlite数据库
 */
void sqlite_close()
{
	if (NULL != g_db) {
		sqlite3_close(g_db);
		g_db = NULL;
	}
}

