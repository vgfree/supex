#pragma once

#include <libmini.h>

/*==================================================*
 *       free queue                                  *
 *===================================================*/
/** 任务队列*/
struct free_queue_list
{
	unsigned int    max;	/**< 最大容量*/
	unsigned int    all;	/**< 最大空间*/
	unsigned int    dsz;	/**< 每个元素的大小*/
	unsigned int    isz;	/**< 累计压入*/
	unsigned int    osz;	/**< 累计弹出*/
	AO_T            tasks;	/**< 任务数量,tasks > 0 时代表有新增任务*/
	void            *slots;	/**< 队列BUFFER*/

	unsigned int    head;	/**< 当前队列头槽位号*/
	unsigned int    tail;	/**< 当前队列尾槽位号*/

	AO_SpinLockT    w_lock;	/**< 压入锁*/
	AO_SpinLockT    r_lock;	/**< 弹出锁*/
};

/**
 * 初始化
 * @param list 被初始化队列
 * @param dsz  元素大小
 * @param max  最大容量
 */
void free_queue_init(struct free_queue_list *list, unsigned int dsz, unsigned int max);

/**
 * 将元素的副本压入队列尾部
 * @param list 操作队列
 * @param data 被压入队列的元素
 * @return 是否推入成功true/false
 */
bool free_queue_push(struct free_queue_list *list, void *data);

/**
 * 从队列的头部弹出一个元素的副本
 * @param list 操作队列
 * @param data 被弹出队列的元素
 * @return 是否获取成功true/false
 */
bool free_queue_pull(struct free_queue_list *list, void *data);

/**
 * 查看队列头数据
 */
bool free_queue_top(struct free_queue_list *list, void *data);

/**
 * 遍历队列
 */
void free_queue_travel(struct free_queue_list *list, bool (*travel)(void *, size_t size, void *), void *data);

