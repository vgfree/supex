/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_LIST_H__
#define __COMM_LIST_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include	"comm_utils.h"

#ifndef container_of
/*
*   * container_of - cast a member of a structure out to the containing structure
*   * @ptr: the pointer to the member.
*   * @type: the type of the container struct this is embedded in.
*   * @member: the name of the member within the struct.
*/
#define container_of(ptr, type, member) ({ \
		 const typeof( ((type *)0)->member ) *__mptr = (ptr); \
		 (type *)( (char *)__mptr - offsetof(type,member) );})
#endif


/* 获取一个结构体里面成员变量的偏移 @ptr：结构体的地址 @member：成员变量的地址 */
#define	get_member_offset(ptr, member)	((intptr_t)member - (intptr_t)ptr)

/* 获取一个结构体的地址：@member：结构体里面的成员， @offset：成员变量在结构体里面的偏移 */
#define get_container_addr(member, offset)	((intptr_t)member - (intptr_t)offset)


/* 包含链表结构体的数据销毁回调函数 */
typedef void (*DestroyCB)(void *data);
     
/* 链表结构体 */
struct comm_list {
	DestroyCB	destroy;	/* 销毁数据的回调函数 */
	int		nodes;		/* 链表中有效数据的节点数 */
	struct comm_list* next;		/* 指向链表中的下一个节点 */
	struct comm_list* tail;		/* 指向链表的尾节点 */
};

/* 初始化链表，设置链表的头节点 */
static inline void commlist_init(struct comm_list *list, DestroyCB destroy) {
	assert(list);
	list->tail = list;
	list->next = list;
	list->nodes = 0;
	list->destroy = destroy;
}

/* @head:头节点 @list:要插入的节点 */
static inline bool commlist_push(struct comm_list *head, struct comm_list *list)
{
	head->tail->next = list;
	head->tail = list;
	head->tail->tail = head->tail;
	head->tail->next = head;
	head->nodes += 1;
	return true;
}

/* 取出一个节点的数据 @head:头节点 @返回值：取出节点 */
static inline bool commlist_pull(struct comm_list *head, struct comm_list **data)
{
	struct comm_list *list = head->next;
	head->next = head->next->next;
	if (list != head ) {
		if (list == head->tail) {
			head->tail = head;
		}
		*data = list;
		head->nodes -= 1;
		return true;
	} else {
		return false;
	}
}

/* 销毁嵌套链表的结构体数据： @head：链表的头节点 @offset：链表在嵌套其结构体中的偏移 */
static inline void commlist_destroy(struct comm_list *head, int offset){
	struct comm_list *list = head->next;
	if (head->destroy && head && offset > 0) {
		while (list != head ) {
			head->destroy((void *)get_container_addr(list, offset));
			list = list->next;
		}
	}
}




#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_LIST_H__ */
