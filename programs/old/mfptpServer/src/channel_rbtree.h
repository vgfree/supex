#pragma once

#include "user_record_def.h"

/* 名      称: rb_insert_channel_record
 * 功      能: 插入用户
 * 参      数:
 * 返回值:  TRUE表示成功，FALSE表示失败
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_insert_channel_record(rb_root *root, CHANNEL_RECORD *user);

/* 名      称: rb_remove_channel_record
 * 功      能:
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_remove_channel_record(rb_root *root, char *channel);

/* 名      称: rb_search_channel_record
 * 功      能: 根据用户标识符，查找用户
 * 参      数:
 * 返回值:  成功返回user_info 结构体指针，失败返回NULL
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
CHANNEL_RECORD *rb_search_channel_record(rb_root *root, char *channel);

extern rb_root g_channel_info_tree;

