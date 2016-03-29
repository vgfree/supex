#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "memory_bath.h"

struct mem_list *membt_init(int type_size, unsigned long peak_count)
{
	struct mem_list *list = calloc(1, sizeof(struct mem_list));

	assert(list);

	printf("\x1B[0;32mPEAK_COUNT\t=\t\x1B[1;31m%zu\n\x1B[m", peak_count);
	unsigned long   page_size = getpagesize();
	unsigned long   bath_size = page_size * MEM_STEP_PAGE_COUNT;
	unsigned long   step_count = bath_size / type_size;

	list->type_size = type_size;
	list->peak_count = peak_count;
	list->step_count = step_count;
	list->max_index = -1;
	list->free_list_head = NULL;
	list->ptr_slot = calloc(GET_NEED_COUNT(peak_count, step_count), sizeof(uintptr_t *));
	assert(list->ptr_slot);

	return list;
}

/*
 * 函数名:membt_gain
 * 功能:获取mem_list中第index处的内存
 * 参数:index 从０开始
 */
uintptr_t *membt_gain(struct mem_list *list, unsigned long index)
{
	// printf("%llu\n", index);
	assert(index >= 0 && index < list->peak_count);

	if ((list->max_index < 0) || (list->max_index < index)) {
		list->max_index = index;
	}

	unsigned long   divisor = (index + 1) / list->step_count;
	unsigned long   remainder = (index + 1) % list->step_count;
	unsigned long   offset = (divisor + ((remainder >= 1) ? 1 : 0)) - 1;
	uintptr_t       **base = &list->ptr_slot[offset];

	if (*base == NULL) {
		*base = calloc(MEM_STEP_PAGE_COUNT, getpagesize());
		assert(*base);
	}

	if (remainder == 0) {
		remainder = list->step_count;
	}

	remainder--;
	uintptr_t *addr = (uintptr_t *)&(((char *)*base)[list->type_size * remainder]);
	return addr;
}

int membt_good(struct mem_list *list, unsigned long index)
{
	if ((index < 0) || (index >= list->peak_count)) {
		return !!(NULL);
	}

	assert(index >= 0 && index < list->peak_count);

	unsigned long offset = GET_NEED_COUNT(index + 1, list->step_count) - 1;

	return !!list->ptr_slot[offset];
}

/*
 * 函数名:membt_get_free
 * 功能:从空闲链表中获取一个空间内存的地址
 * 返回值:如果空闲链表为空，则返回ＮＵＬＬ，否则返回一个空闲内存块的地址
 */
uintptr_t *membt_get_free(struct mem_list *list)
{
	if (NULL == list->free_list_head) {
		return NULL;
	}

	uintptr_t *res = NULL;
	res = (uintptr_t *)list->free_list_head;
	list->free_list_head = list->free_list_head->next;
	return res;
}

/*
 * 函数名:membt_free_mem
 * 功能:把已用的内存块中某块要停止使用的内容添加到空闲链表中
 * 参数:address 指向要添加的那块内存地址, list 指向管理内存池的数据结构
 */
void membt_free_mem(struct mem_list *list, uintptr_t *address)
{
	if ((NULL != list) && (NULL != address)) {
		struct free_node node = { NULL };
		memcpy((char *)address, (char *)&node, sizeof(struct free_node));

		if (NULL == list->free_list_head) {
			list->free_list_head = (struct free_node *)address;
		} else {
			((struct free_node *)address)->next = list->free_list_head;
			list->free_list_head = (struct free_node *)address;
		}
	}
}

/*
 * 函数名:membt_delete_all_mem
 * 功能:删除所有内存
 */
void membt_delete_all_mem(struct mem_list *list)
{
	if (NULL != list) {
		unsigned long   divisor = (list->max_index + 1) / list->step_count;
		unsigned long   remainder = (list->max_index + 1) % list->step_count;
		divisor = divisor + (remainder > 0 ? 1 : 0) - 1;

		int i = 0;

		for (; i <= divisor; i++) {
			if (NULL != list->ptr_slot[i]) {
				free(list->ptr_slot[i]);
			}
		}

		free(list->ptr_slot);
		free(list);
	}
}

