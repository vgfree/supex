/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_LIST_H__
#define __COMM_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "comm_utils.h"
#include <stdint.h>
#include <stddef.h>

#ifndef TRAVEL_FCB
typedef void (*TRAVEL_FCB)(const void *data, size_t size, size_t idx, void *usr);
#endif

/* 链表节点结构点 */
struct comm_node
{
	struct comm_node        *next;		/* 后置节点 */
	struct comm_node        *prev;		/* 前置节点 */
	size_t                  size;
	char                    data[0];	/* 节点所保存的数据 */
};

/* 链表的结构体 */
struct comm_list
{
	bool                    init;
	struct comm_node        *head;		/* 链表的头节点 */
	struct comm_node        *tail;		/* 链表的尾节点 */
	unsigned long           nodes;		/* 链表中节点总数 */
	pthread_spinlock_t      lock;		/* 压入锁 */
};

/* 初始化链表，设置链表的头节点 */
void commlist_init(struct comm_list *list);

/* 销毁嵌套链表的结构体数据： @list：链表的结构体*/
void commlist_destroy(struct comm_list *list, TRAVEL_FCB fcb, void *usr);

/* 从尾部插入一个新节点 @list:链表结构体*/
bool commlist_push(struct comm_list *list, void *data, size_t size);

/* 取出链表第一个节点的数据 @list:链表结构体  @node:存放取到的节点地址 */
bool commlist_pull(struct comm_list *list, void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif	/* ifndef __COMM_LIST_H__ */

