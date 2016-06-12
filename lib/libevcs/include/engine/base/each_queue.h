#pragma once

#include <libmini.h>

/*==================================================*
 *       free queue                                  *
 *===================================================*/
/** 任务队列*/
struct each_queue_list
{
	unsigned int    max;	/**< 最大容量*/
	unsigned int    all;	/**< 最大空间*/
	unsigned int    dsz;	/**< 每个元素的大小*/
	unsigned int    isz;	/**< 累计压入*/
	unsigned int    osz;	/**< 累计弹出*/
	AO_T            tasks;	/**< 任务数量,tasks > 0 时代表有新增任务*/
	void            *slots;	/**< 队列BUFFER*/
	unsigned int    *marks;	/**< 任务消费计数*/

	unsigned int    head;	/**< 当前队列头槽位号*/
	unsigned int    tail;	/**< 当前队列尾槽位号*/

	AO_SpinLockT    w_lock;	/**< 压入锁*/
	AO_SpinLockT    r_lock;	/**< 弹出锁*/
	
	
	unsigned int	use;	/**< 消费者数*/
	unsigned int    *shift;	/**< 用户消费记录*/
};

/**
 * 初始化
 * @param list 被初始化队列
 * @param dsz  元素大小
 * @param max  最大容量
 */
void each_queue_init(struct each_queue_list *list, unsigned int dsz, unsigned int max, unsigned int use);

/**
 * 将元素的副本压入队列尾部
 * @param list 操作队列
 * @param data 被压入队列的元素
 * @return 是否推入成功true/false
 */
bool each_queue_push(struct each_queue_list *list, void *data);

/**
 * 从队列的头部弹出一个元素的副本
 * @param list 操作队列
 * @param data 被弹出队列的元素
 * @return 是否获取成功true/false
 */
bool each_queue_pull(struct each_queue_list *list, void *data, unsigned int index);


/**
 * 释放队列
 */
void each_queue_free(struct each_queue_list *list);
