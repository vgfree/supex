#pragma once

#include <sqlite3.h>
#include <stdint.h>
#include "basic_type.h"

struct sql_info
{
	char    *database;
	char    *table_name;
	char    *sql;
};

/*
 * 函数名:sqlite_init
 * 功能:创建数据
 * 参数:db_name数据库名
 * 返回值:成功返回TRUE,失败返回FALSE
 */

int64_t sqlite_init(struct sql_info *info);

/*
 * arg:从主函数传进来的参数
 * values:是查询结果的值
 * names:查询结果列的名字
 */
typedef int (*SQLITE_CALLBACK)(void *arg, int nr, char **values, char **names);

/*
 * 函数名:sqlite_cmd
 * 功能：调用sqlite执行sql语句，对返回结果调用cb回调函数进行处理
 * 参数:sql 要执行的sql语句,cb 回调函数
 * 返回值:成功返回ＴＲＵＥ，失败返回ＦＡＬＳＥ
 */
int64_t sqlite_cmd(struct sql_info *sql, SQLITE_CALLBACK cb);

/*
 * 函数名:sqlite_close
 * 功能:关闭sqlite数据库
 */
void sqlite_close();

