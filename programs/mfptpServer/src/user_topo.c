/*
 * topo.c
 *
 *  Created on: Jun 19, 2014
 *      Author: buyuanyuan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <inttypes.h>

#include "user_topo.h"
#include "memory_bath.h"
#include "user_rbtree.h"
#include "channel_rbtree.h"

struct sql_info         g_sql_info;
static struct mem_list  *g_user_record_list = NULL;	// 存放数据库记录信息的内存池

static void _insert_user_tree(USER_RECORD *p_user_record);

static void _insert_channel_tree(USER_RECORD *p_channel_record);

/*
 * 函数名:topo_push_user_record
 * 功能:把obj以内存复制的方式存到内存池的idx位置
 */
void _topo_push_user_record(USER_RECORD *obj, uint64_t idx);

/*
 * 函数:topo_data_mem_init
 * 功能:初始化g_user_record_list
 * 返回:void
 */
void topo_data_mem_init(void)
{
	if (NULL == g_user_record_list) {
		g_user_record_list = membt_init(sizeof(USER_RECORD), MAX_TOPO_USER_RECORD_COUNT);
	}
}

/*
 * 名称:topo_fetch_data_list
 * 功能:获取g_user_record_list
 * 返回值:返回g_user_record_list
 */
struct mem_list *topo_fetch_data_list()
{
	return g_user_record_list;
}

/*
 * 函数名:topo_push_user_record
 * 功能:以内存复制的方式把obj存放到idx位置上
 */
void _topo_push_user_record(USER_RECORD *obj, uint64_t idx)
{
	USER_RECORD *p_user_record = (USER_RECORD *)membt_gain(g_user_record_list, idx);

	memcpy(p_user_record, obj, sizeof(USER_RECORD));
	_insert_user_tree(p_user_record);
	_insert_channel_tree(p_user_record);
}

/*
 * 函数名:_insert_user_tree
 *  功能:把结点插入到用户红黑树中
 */
static void _insert_user_tree(USER_RECORD *p_user_record)
{
	// 1:先把用户记录插入到用户红黑树中
	USER_RECORD *p_user_first_record = NULL;

	// 1.1:先在用户红黑树中找到该用户的第一条记录
	p_user_first_record = rb_search_user_record(&g_user_info_tree, p_user_record->user_id);

	// 1.1.1在红黑树中没有找到，则当前是第一条记录
	if (NULL == p_user_first_record) {
		// 1.1.1.1把当前结点插入到红黑树中
		rb_insert_user_record(&g_user_info_tree, p_user_record);
		p_user_record->user_next_record = NULL;
		p_user_record->user_prev_record = NULL;
	}
	// 1.1.2在红黑树中找到用户的第一条记录，把当前记录挂在第一条记录的下面
	else {
		p_user_record->user_next_record = p_user_first_record->user_next_record;

		if (NULL != p_user_record->user_next_record) {
			p_user_record->user_next_record->user_prev_record = p_user_record;
		}

		p_user_first_record->user_next_record = p_user_record;
		p_user_record->user_prev_record = p_user_first_record;
		// 用户的这条记录的fd设置为和第一条记录的fd一样
		p_user_record->user_data = p_user_first_record->user_data;
	}
}

/*
 * 函数名:_insert_channel_tree
 * 功能:把频道信息插入到频道红黑树中
 */
static void _insert_channel_tree(USER_RECORD *p_channel_record)
{
	// 1:把频道记录插入到频道红黑树中
	USER_RECORD *p_channel_first_record = NULL;

	// 1.1先在频道红黑树中找到该频道的第一条记录
	p_channel_first_record = rb_search_channel_record(&g_channel_info_tree, p_channel_record->channel);

	// 1.1.1:没有找到，则该记录为该频道的第一条记录
	if (NULL == p_channel_first_record) {
		rb_insert_channel_record(&g_channel_info_tree, p_channel_record);
		p_channel_record->channel_next_record = NULL;
		p_channel_record->channel_prev_record = NULL;
	} else {
		// 1.1.2找到第一条记录，把当前挂在第一条记录的下面
		p_channel_record->channel_next_record = p_channel_first_record->channel_next_record;

		if (NULL != p_channel_record->channel_next_record) {
			p_channel_record->channel_next_record->channel_prev_record = p_channel_record;
		}

		p_channel_record->channel_prev_record = p_channel_first_record;
		p_channel_first_record->channel_next_record = p_channel_record;
	}
}

#define SQL_DELETE_USER_RECORD "delete from %s where user_imei = %lu and channel = %s"

/*
 * 函数:topo_delete_user_record
 * 功能:把频道中的某一用户删除
 * 参数:channel频道,imei要删除的用户
 * 返回值:TRUE删除成功，FALSE删除失败
 */
int topo_delete_user_record(char *channel, char *im)
{
	int res = FALSE;

	if ((NULL == channel) || (NULL == im)) {
		return res;
	}

	uint64_t imei = atol(im);
	// 1:从内存中查找到该频道
	CHANNEL_RECORD  *p_channel_first_record = NULL;
	CHANNEL_RECORD  *p_channel_record = NULL;	// 匹配channel 和imei的记录

	if (NULL != channel) {
		p_channel_first_record = rb_search_channel_record(&g_channel_info_tree, channel);
		p_channel_record = p_channel_first_record;

		if (NULL != p_channel_record) {
			// 检查该频道的这个记录是否为该用户，如果不是，则向后查找（不用向前查找是因为该记录是该频道的第一条记录）
			while (p_channel_record) {
				if (imei == p_channel_record->user_id) {
					res = TRUE;
					break;
				}

				p_channel_record = p_channel_record->channel_next_record;
			}
		}
	}

	// 2:从内存中移除该记录的频道关系　
	if (TRUE == res) {
		// 2.1频道的第一条记录就是要查找的记录
		if (p_channel_first_record == p_channel_record) {
			// 2.1.1该频道就这一条记录，则把第一条记录直接从红黑树中删除
			if (NULL == p_channel_first_record->channel_next_record) {
				rb_remove_channel_record(&g_channel_info_tree, channel);
			}
			// 2.1.2该频道有多条记录,则把第一条记录从红黑树中删除，把频道的第二条记录插入红黑树，同时修改第二条记录的链表指针
			else {
				rb_remove_channel_record(&g_channel_info_tree, channel);
				rb_insert_channel_record(&g_channel_info_tree, p_channel_record->channel_next_record);
				p_channel_record->channel_prev_record = NULL;
			}
		}
		// 2.2频道的第一条记录不是要查找的记录,把找到的记录从该频道的双链表结构中移出
		else {
			p_channel_record->channel_prev_record->channel_next_record = p_channel_record->channel_next_record;

			// 该记录不是最后一条该频道的记录
			if (NULL != p_channel_record->channel_next_record) {
				p_channel_record->channel_next_record->channel_prev_record = p_channel_record->channel_prev_record;
			}
		}
	}

	// 3:从内存中把该记录从用户的关系的移出
	if (TRUE == res) {
		// 3.1判断该条记录中的用户信息是否有前趋，以判断该条记录是否为该用户的第一条记录
		// 3.1.1该记录是用户的第一条记录
		if (NULL == p_channel_record->user_prev_record) {
			// 3.1.1.1 该用户是没有下一条记录,直接把该记录从红黑树中移出
			if (NULL == p_channel_record->user_next_record) {
				rb_remove_user_record(&g_user_info_tree, p_channel_record->user_id);
			}
			// 3.1.1.2　该用户有下一条记录,把第一条记录从红黑树中移出，并把第二条记录添加到红黑树中同时设置第二条记录的前趋和后趋
			else {
				rb_remove_user_record(&g_user_info_tree, p_channel_record->user_id);
				rb_insert_user_record(&g_user_info_tree, p_channel_record->user_next_record);
				p_channel_record->user_next_record->user_prev_record = NULL;
				// 把下一条的用户fd设置成第一条记录的fd
				p_channel_record->user_next_record->user_data = p_channel_record->user_data;
			}
		}
		// 3.1.2 该记录不是用户的第一条记录
		else {
			p_channel_record->user_prev_record->user_next_record = p_channel_record->user_next_record;

			// 该记录不是用户的最后一条记录
			if (NULL != p_channel_record->user_next_record) {
				p_channel_record->user_next_record->user_prev_record = p_channel_record->user_prev_record;
			}
		}
	}

	// 4:把删除结点的位置记录到一个空闲空间链表中，以便以后使用
	if (TRUE == res) {
		membt_free_mem(topo_fetch_data_list(), (uintptr_t *)p_channel_record);
	}

	// 5:第一步执行成功，则对mysql数据库进行操作
	if (TRUE == res) {
		char sql[256] = { 0 };
		sprintf(sql, SQL_DELETE_USER_RECORD, g_sql_info.table_name, imei, channel);
		g_sql_info.sql = sql;
		sqlite_cmd(&g_sql_info, NULL);
	}

	return res;
}

#define SQL_INSERT_NEW_RECORD "insert into %s (user_imei, channel) values(%lu, %s)"

/*
 * 函数:topo_add_user_record
 * 功能:把频道channel,imei这条记录添加到组织结构中
 * 参数:table_name 表名，channel记录的频道号，　imei　记录的用户
 * 返回值:成功返回TRUE, 失败返回FALSE
 */
int topo_add_user_record(char *channel, char *im)
{
	int res = FALSE;

	if ((NULL == channel) || (NULL == im)) {
		return res;
	}

	uint64_t        imei = atol(im);
	CHANNEL_RECORD  *p_channel_first_record = rb_search_channel_record(&g_channel_info_tree, channel);
	USER_RECORD     *p_user_first_record = rb_search_user_record(&g_user_info_tree, imei);

	// 1:检查是否已经存在具有相同信息的记录
	if ((NULL != p_channel_first_record) && (NULL != p_user_first_record)) {
		CHANNEL_RECORD *p_channel_record = p_channel_first_record;

		while (p_channel_record) {
			if (imei == p_channel_record->user_id) {
				return res;
			}

			p_channel_record = p_channel_record->channel_next_record;
		}
	}

	// 2:把内容复制到内存池中
	USER_RECORD *address = (USER_RECORD *)membt_get_free(topo_fetch_data_list());

	if (NULL == address) {
		address = (USER_RECORD *)membt_gain(g_user_record_list, topo_fetch_data_list()->max_index + 1);
	}

	USER_RECORD obj;
	memset(&obj, 0, sizeof(USER_RECORD));
	obj.user_id = imei;
	memcpy(obj.channel, channel, strlen(channel));
	obj.user_data = NULL;
	memcpy(address, &obj, sizeof(USER_RECORD));
	// 3:把结点挂到用户树上
	_insert_user_tree(address);
	// 4:把结点挂到频道树上
	_insert_channel_tree(address);

	// 3:更新数据库
	char sql[256] = { 0 };
	sprintf(sql, SQL_INSERT_NEW_RECORD, g_sql_info.table_name, imei, channel);
	g_sql_info.sql = sql;
	sqlite_cmd(&g_sql_info, NULL);

	res = TRUE;
	return res;
}

/*=========================================================================*/

/*
 * 函数:topo_get_user_data
 * 功能:根据用户获取fd
 * 参数:imei 用户IMEI
 */
void *topo_get_user_data(char *im)
{
	if (NULL == im) {
		return NULL;
	}

	uint64_t        imei = atol(im);
	void            *res = NULL;
	USER_RECORD     *p_user_first_record = rb_search_user_record(&g_user_info_tree, imei);

	if (NULL != p_user_first_record) {
		res = p_user_first_record->user_data;
	}

	return res;
}

/*
 * 函数:topo_set_user_data
 * 功能:设置用户的fd
 * 返回值:成功返回TRUE,失败返回FALSE
 */
int topo_set_user_data(char *im, void *data)
{
	if ((NULL == im) || (NULL == data)) {
		return FALSE;
	}

	uint64_t        imei = atol(im);
	USER_RECORD     *p_user_first_record = rb_search_user_record(&g_user_info_tree, imei);

	if (NULL != p_user_first_record) {
		p_user_first_record->user_data = data;
		USER_RECORD *p_user_record = p_user_first_record->user_next_record;

		while (p_user_record) {
			p_user_record->user_data = data;
			p_user_record = p_user_record->user_next_record;
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 * 函数名:fetch_user_record_cb
 * 功能:加载数据库信息时，对数据库中每行进行处理的回调函数
 * arg:从主函数传进来的参数
 * nr 返回的结果行数
 * values:是查询结果的值
 * names:查询结果列的名字
 */
int fetch_user_record_cb(void *arg, int nr, char **values, char **names)
{
	USER_RECORD node;

	if (nr > 0) {
		memset(&node, 0, sizeof(USER_RECORD));
		node.user_id = atol(values[1]);
		// 这里测试时有可能会问题
		memcpy(node.channel, values[2], strlen(values[2]));
		node.user_data = NULL;
		_topo_push_user_record(&node, g_user_record_list->max_index + 1);	// 这里为什么用max_index而不用数据库中的id，是因为数据库中的记录如果删除之后会导致id不一致
	}

	return 0;
}

#define SQL_CREATE_TABLE        "create table %s (id INTEGER PRIMARY KEY NOT NULL, user_imei INTEGER, channel CHAR[20])"
#define SQL_USER_RECORD_QUERY   "SELECT * from %s"

void topo_load_user_record(struct sql_info *info)
{
	topo_data_mem_init();

	// 1:创建或打开数据库
	sqlite_init(info);
	// 2:创建表，创建失败，说明已经创建过,创建成功说明是新表，里面还没有数据
	char sql_buf[256] = { 0 };
	sprintf(sql_buf, SQL_CREATE_TABLE, info->table_name);
	info->sql = sql_buf;
	g_sql_info.database = info->database;
	g_sql_info.table_name = info->table_name;
	g_sql_info.sql = NULL;

	if (TRUE == sqlite_cmd(info, NULL)) {
		return;
	}

	memset(sql_buf, 0, sizeof(sql_buf));
	sprintf(sql_buf, SQL_USER_RECORD_QUERY, info->table_name);
	info->sql = sql_buf;
	sqlite_cmd(info, fetch_user_record_cb);
}

/*
 * 函数名:topo_select_user_by_channel
 * 功能:根据channel查找属于该频道的用户，然后针对每个用户调用回调函数cb
 */
void topo_select_user_by_channel(char *channel, USER_CALLBACK cb)
{
	if ((NULL != channel) && (NULL != cb)) {
		CHANNEL_RECORD *p_channel_record = rb_search_channel_record(&g_channel_info_tree, channel);

		while (p_channel_record != NULL) {
			cb(p_channel_record);
			p_channel_record = p_channel_record->channel_next_record;
		}
	}
}

/*
 * 函数名:topo_select_online_user_by_channel
 * 功能:根据channel查找属于该频道的所有在线用户，然后针对每个用户调用回调函数cb
 */
void topo_select_online_user_by_channel(char *channel, USER_CALLBACK cb)
{
	if ((NULL != channel) && (NULL != cb)) {
		CHANNEL_RECORD *p_channel_record = rb_search_channel_record(&g_channel_info_tree, channel);

		while (p_channel_record != NULL) {
			if (p_channel_record->user_data != NULL) {
				cb(p_channel_record);
				p_channel_record = p_channel_record->channel_next_record;
			} else {
				p_channel_record = p_channel_record->channel_next_record;
			}
		}
	}
}

/*
 * 函数名:topo_select_channel_by_user
 * 功能:根据imei查找用户订阅的频道,对每个频道调用回调函数
 * 参数:imei要查找的用户，　cb 频道的回调函数
 */
void topo_select_channel_by_user(char *im, CHANNEL_CALLBACK cb)
{
	if ((NULL == cb) || (NULL == im)) {
		return;
	}

	uint64_t        imei = atol(im);
	USER_RECORD     *p_user_record = rb_search_user_record(&g_user_info_tree, imei);

	while (p_user_record) {
		cb(p_user_record);
		p_user_record = p_user_record->user_next_record;
	}
}

/*
 * 函数名:topo_free
 * 功能:释放所有内存和数据库连接
 */
void topo_free()
{
	if (NULL != g_user_record_list) {
		membt_delete_all_mem(g_user_record_list);
		sqlite_close();
	}
}

