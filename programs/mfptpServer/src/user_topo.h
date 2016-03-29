/*
 * user_topo.h
 *
 *  Created on: Jun 19, 2014
 *      Author: buyuanyuan
 */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "user_record_def.h"
#include "sqlite_api.h"
#include "basic_type.h"

#define MAX_TOPO_USER_RECORD_COUNT      100000000
#define MAX_ONE_NODE_OWN_LINE_COUNT     8

/*
 * 函数名:topo_data_init
 * 功能:初始化内存池
 */
void topo_data_mem_init();

/*
 * 函数名:topo_fetch_data_list
 * 功能:返回管理内存池的数据结构
 */
struct mem_list *topo_fetch_data_list();

/*
 * 函数:topo_load_user_record
 * 功能:从数据库中读取数据插入到内存池中
 */
void topo_load_user_record(struct sql_info *info);

/*
 * 函数:topo_delete_user_record
 * 功能:把频道中的某一用户删除
 * 参数:channel频道,imei要删除的用户
 * 返回值:TRUE删除成功，FALSE删除失败
 */
int topo_delete_user_record(char *channel, char *imei);

/*
 * 函数:topo_add_user_record
 * 功能:把频道channel,imei这条记录添加到组织结构中
 * 参数:info 数据库信息,channel记录的频道号，　imei　记录的用户
 * 返回值:成功返回TRUE, 失败返回FALSE
 */
int topo_add_user_record(char *channel, char *imei);

/*
 * 函数:topo_add_new_user_record
 * 功能:把新用户的记录插入到组织结构中
 * 参数:info数据库信息，　channel 记录的频道号,imei记录的用户，　fd 用户的新记录
 *
 *   int topo_add_new_user_record(struct sql_info* info, char* channel, uint64_t imei, int64_t fd);
 */

typedef  void (*USER_CALLBACK)(USER_RECORD *);

/*
 * 函数:topo_get_user_data
 * 功能:根据用户获取fd
 * 参数:imei 用户IMEI
 */
void *topo_get_user_data(char *imei);

/*
 * 函数:topo_set_user_data
 * 功能:设置用户的data信息
 * 返回值:成功返回TRUE,失败返回FALSE
 */
int topo_set_user_data(char *imei, void *);

/*
 * 函数名:topo_select_user_by_channel
 * 功能:根据channel查找属于该频道的用户，然后针对每个用户调用回调函数cb
 */
void topo_select_user_by_channel(char *channel, USER_CALLBACK cb);

/*
 * 函数名:topo_select_online_user_by_channel
 * 功能:根据channel查找属于该频道的所有在线用户，然后针对每个用户调用回调函数cb
 */
void topo_select_online_user_by_channel(char *channel, USER_CALLBACK cb);

typedef void (*CHANNEL_CALLBACK)(CHANNEL_RECORD *);

/*
 * 函数名:topo_select_channel_by_user
 * 功能:根据imei查找用户订阅的频道,对每个频道调用回调函数
 * 参数:imei要查找的用户，　cb 频道的回调函数
 */
void topo_select_channel_by_user(char *imei, CHANNEL_CALLBACK cb);

/*
 * 函数名:topo_free
 * 功能:释放所有内存和数据库连接
 */
void topo_free();

