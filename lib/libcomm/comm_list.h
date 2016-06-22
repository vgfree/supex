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

/* 链表结构体 */
struct comm_list
{
	DestroyCB               destroy;	/* 销毁数据的回调函数 */
	int                     nodes;		/* 链表中有效数据的节点数[只有头节点会被设置] */
	struct comm_list        *next;		/* 指向链表中的下一个节点 */
	struct comm_list        *tail;		/* 指向链表的尾节点[只有头节点会被设置] */
	struct comm_list	*cur;		/* 指向当前的节点数[用于依次获取链表节点而不删除节点数据只有头节点会被设置]*/
};

/* 初始化链表，设置链表的头节点 */
static inline void commlist_init(struct comm_list *list, DestroyCB destroy)
{
	assert(list);
	list->cur = list;
	list->tail = list;
	list->next = list;
	list->nodes = 0;
	list->destroy = destroy;
}

/* 从尾部插入一个新节点 @head:头节点 @list:要插入的节点 */
static inline bool commlist_push(struct comm_list *head, struct comm_list *list)
{
	assert(head && list);
	head->tail->next = list;
	head->tail = list;
	head->tail->next = head;
	head->nodes += 1;
	return true;
}

/* 取出链表第一个节点的数据 @head:头节点 @list:存放取到的节点地址 */
static inline bool commlist_pull(struct comm_list *head, struct comm_list **list)
{
	assert(head && list);
	struct comm_list *temp = head->next;

	head->next = head->next->next;

	if (temp != head) {
		if (temp == head->tail) {
			head->tail = head;
		}

		*list = temp;
		head->nodes -= 1;
		return true;
	} else {
		*list = NULL;
		return false;
	}
}

/* 依次获得链表的数据 但并不将节点从链表删除 */
static inline bool commlist_get(struct comm_list *head, struct comm_list **list)
{
	assert(head && list);
	head->cur = head->cur->next;
	if (head->cur != head) {
		*list = head->cur;
		return true;
	} else {
		*list = NULL;
		return false;
	}
}

/* 删除链表中指定的节点 @head:链表头节点 @list:需要删除的节点地址 */
static inline bool commlist_delete(struct comm_list *head, struct comm_list *list)
{
	assert(head && list);
	struct comm_list *cur = head->next;
	struct comm_list *pre = head;
	while (cur != head) {
		if (cur == list) {
			if (cur == head->tail) {
				head->tail = pre;
			}
			pre->next = cur->next;
			head->nodes -= 1;
			return true;
		}
		pre = cur;
		cur = cur->next;
	}
	return false;
}

/* 销毁嵌套链表的结构体数据： @head：链表的头节点 @offset：链表在嵌套其结构体中的偏移 */
static inline void commlist_destroy(struct comm_list *head, int offset)
{
	struct comm_list *list = NULL;

	if (head->destroy && head && (offset > 0)) {
		while (commlist_pull(head, &list)) {
			head->destroy((void *)get_container_addr(list, offset));
		}
	}
}

#ifdef __cplusplus
}
#endif

#endif	/* ifndef __COMM_LIST_H__ */

