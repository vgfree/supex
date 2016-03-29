#pragma once

#include <stdint.h>

#define MEM_STEP_PAGE_COUNT 32
#define GET_NEED_COUNT(idx, div) ((idx) / (div) + (((idx) % (div) >= 1) ? 1 : 0))

struct free_node
{
	struct free_node *next;
};

struct mem_list
{
	int                     type_size;
	unsigned long           peak_count;		/*max of full members*/
	unsigned long           step_count;		/*max of bath members*/
	uintptr_t               **ptr_slot;
	long                    max_index;		// 记录当前已用内存块的最大索引,在数据初始化之后的数据再次插入有用。初始化值为-1,
	struct free_node        *free_list_head;	// 把删除的空闲内存用单链表的形式串起来
};

struct mem_list *membt_init(int type_size, unsigned long peak_count);

int membt_good(struct mem_list *list, unsigned long index);

uintptr_t *membt_gain(struct mem_list *list, unsigned long index);

/*
 * 函数名:membt_get_free
 * 功能:从空闲链表中获取一个空间内存的地址
 * 返回值:如果空闲链表为空，则返回ＮＵＬＬ，否则返回一个空闲内存块的地址
 */
uintptr_t *membt_get_free(struct mem_list *list);

/*
 * 函数名:membt_free_mem
 * 功能:把已用的内存块中某块要停止使用的内容添加到空闲链表中
 * 参数:address 指向要添加的那块内存地址
 *
 */
void membt_free_mem(struct mem_list *list, uintptr_t *address);

/*
 * 函数名:membt_delete_all_mem
 * 功能:删除所有内存
 */
void membt_delete_all_mem(struct mem_list *list);

