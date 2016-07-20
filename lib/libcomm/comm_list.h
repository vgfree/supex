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

#ifndef container_of

/*
 *   * container_of - cast a member of a structure out to the containing structure
 *   * @ptr: the pointer to the member.
 *   * @type: the type of the container struct this is embedded in.
 *   * @member: the name of the member within the struct.
 */
  #define container_of(ptr, type, member)			    \
	({							    \
		const typeof(((type *)0)->member) * __mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member));  \
	})
#endif

/* 获取一个结构体里面成员变量的偏移 @ptr:结构体的地址 @member:成员变量的地址 */
#define get_member_offset(ptr, member)          ((intptr_t)member - (intptr_t)ptr)

/* 根据结构体某个成员变量获取结构体的地址:@member：结构体里面的成员, @offset:成员变量在结构体里面的偏移 */
#define get_container_addr(member, offset)      ((intptr_t)member - (intptr_t)offset)

/* 链表节点所保存的数据销毁回调函数 */
typedef void (*DestroyCB)(void *data);

/* 链表节点结构点 */
struct list_node {
	struct list_node* next;		/* 后置节点 */
	struct list_node* prev;		/* 前置节点[保留值未使用] */
	void* value;			/* 节点所保存的值[保留未使用目前将此结构体嵌套到别的结构体而不是将别的结构体保存在此变量中] */
};

/* 链表的结构体 */
struct comm_list {
	struct list_node* head;		/* 链表的头节点 */
	struct list_node* tail;		/* 链表的尾节点 */
	struct list_node* cur;		/* 指向当前的节点[用于依次获取链表节点数据而不删除节点数据] */
	unsigned long nodes;		/* 链表中节点总数 */
	DestroyCB     destroy;		/* 销毁数据的回调函数 */
	//void (*free)(void* data);	/* 节点值的销毁函数 */

};


/* 初始化链表，设置链表的头节点 */
static inline void commlist_init(struct comm_list *list, DestroyCB destroy)
{
	assert(list);
	list->cur = NULL;
	list->tail = NULL;
	list->head = NULL;
	list->nodes = 0;
	list->destroy = destroy;
}

/* 从尾部插入一个新节点 @list:链表结构体 @node:要插入的节点 */
static inline bool commlist_push(struct comm_list *list, struct list_node *node)
{
	assert(list && node);
	node->next = NULL;
	if (list->head != NULL) {
		list->tail->next = node;
		list->tail = node;
	} else {
		list->head = node;
		list->tail = node;
		list->cur = node;
	}
	list->nodes ++;
	return true;
}

/* 取出链表第一个节点的数据 @list:链表结构体  @node:存放取到的节点地址 */
static inline bool commlist_pull(struct comm_list *list, struct list_node **node)
{
	assert(list && node);
	if (list->nodes > 0) {
		*node = list->head;
		if (list->head == list->tail) {
			/* 取的是最后一个节点数据 */
			list->tail = NULL;
		}
		list->head = list->head->next;
		list->nodes --;
		return true;
	}
	*node = NULL;
	return false;
}

/* 依次获得链表的数据 但并不将节点从链表删除 */
static inline bool commlist_get(struct comm_list *list, struct list_node **node)
{
	assert(list && node);
	if (list->cur != NULL) {
		*node = list->cur;
		list->cur = list->cur->next;
		return true;
	}
	*node = NULL;
	return false;
}

/* 删除链表中指定的节点 @list:链表结构体  @node:需要删除的节点地址 */
static inline bool commlist_delete(struct comm_list *list, struct list_node *node)
{
	assert(list && node);
	struct list_node *cur = list->head;
	struct list_node *pre = list->head;
	while (cur != NULL) {
		if (cur == node) {
			if (cur == list->head) {
				list->head = list->head->next;
			}
			if (cur == list->tail) {
				list->tail = pre;
			}
			pre->next = cur->next;
			list->nodes --;
			return true;
		}
		pre = cur;
		cur = cur->next;
	}
	return false;
}

/* 销毁嵌套链表的结构体数据： @list：链表的结构体 @offset：链表在嵌套其结构体中的偏移 */
static inline void commlist_destroy(struct comm_list *list, int offset)
{
	struct list_node *node = NULL;
	if (list && list->destroy && (offset > 0)) {
		while(commlist_pull(list, &node)) {
			list->destroy((void*)get_container_addr(node, offset));
		}
	}
}


#ifdef __cplusplus
}
#endif

#endif	/* ifndef __COMM_LIST_H__ */
