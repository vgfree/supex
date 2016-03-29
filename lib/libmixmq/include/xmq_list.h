#pragma once

/****************************************************
 *         双向环形链表                             *
 ***************************************************/

/*链表结构*/
typedef struct _list_t
{
	struct _list_t  *prev;
	struct _list_t  *next;
} list_t;

/*list_init - 初始化链表，前后指向自身
 * @head:          需要初始化的表头
 * return:         无
 * */
static inline void list_init(list_t *head)
{
	head->next = head;
	head->prev = head;
}

/*list_empty - 判断链表是否为空
 * @head:      需要初始化的表头
 * return:     为空返回true，否则false
 * */
static inline int list_empty(const list_t *head)
{
	return head->next == head;
}

/*__list_add - 向链表中指定两个相邻节点之间插入新节点
 * @pnew:      将要插入链表的节点
 * @prev:      要插入的两相邻节点中的前节点
 * @next:      要插入的两相邻节点中的后节点
 * return:     无
 * */
static inline void __list_add(list_t *pnew, list_t *prev, list_t *next)
{
	next->prev = pnew;
	pnew->next = next;
	pnew->prev = prev;
	prev->next = pnew;
}

/*__list_del - 从链表中删除三个相邻节点中的中间节点
 * @prev:      三个相邻节点中的前节点
 * @next:      三个相邻节点中的后节点
 * return:     无
 * */
static inline void __list_del(list_t *prev, list_t *next)
{
	next->prev = prev;
	prev->next = next;
}

/*list_del - 删除链表中的某个节点
 * @list:    链表中的某个节点
 * return:   无*/
static inline void list_del(list_t *list)
{
	__list_del(list->prev, list->next);
}

/*list_add_tail - 向链表尾部添加节点
 * @pnew:         需要新增的节点
 * @head:         链表头
 * return:        无
 * */
static inline void list_add_tail(list_t *pnew, list_t *head)
{
	__list_add(pnew, head->prev, head);
}

/*list_add_head - 向链表头部添加节点
 * @pnew:         需要新增的节点
 * @head:         链表头
 * return:        无
 * */
static inline void list_add_head(list_t *pnew, list_t *head)
{
	__list_add(pnew, head, head->next);
}

/*list_for_each - 遍历链表
* @pos:          迭代节点指针
* @head:         链表头*/
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*offsetof - 计算结构体中成员变量在结构体中的偏移
 * @type:    结构体类型
 * @member:  成员变量名
 * return:   从结构体首地址到该成员变量首地址的字节数
 * */
#ifndef offsetof
  #define offsetof(type, member) ((size_t)(char *)&((type *)0)->member)
#endif

/*container_of - 获得包含该成员变量的结构体地址
 * @ptr:         成员变量的地址
 * @type:        结构体类型
 * @member:      成员变量名
 * return:       包含该成员变量的结构体地址
 * */
#ifndef container_of
  #define container_of(ptr, type, member)			    \
	({							    \
		const typeof(((type *)0)->member) * __mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member));  \
	})
#endif

/*list_del_all_entrys - 删除并释放链表中所有节点
 * @head:               该链表头
 * @type:               节点结构类型
 * @member:             链表结构在节点中的成员名
 * return               无*/
#define list_del_all_entrys(head, type, member)			 \
	do {							 \
		list_t *__pos = (head)->next;			 \
		for (; __pos != (head); __pos = (head)->next) {	 \
			list_del(__pos);			 \
			free(container_of(__pos, type, member)); \
		}						 \
	} while (0)

