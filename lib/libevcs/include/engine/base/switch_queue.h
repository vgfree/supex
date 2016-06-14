#pragma once

#include "supex.h"
#include "base/free_queue.h"

struct switch_queue_info;

typedef bool (*QUEUE_ENTITY_CALL)(struct switch_queue_info *p_stat, struct supex_task_node *p_node, va_list *ap);

#define MAX_SWITCH_QUEUE_SWAP_SIZE 2		// must >= 2
enum
{
	MARK_USE_MAJOR = 0,
	MARK_USE_MINOR,
};
struct switch_queue_info
{
	int                     mark_report;
	unsigned int            shift_report;
	int                     mark_lookup;
	unsigned int            shift_lookup;
	int                     step_lookup;

	struct supex_task_node  temp;
	struct free_queue_list  swap;

	volatile long           *major_have;
	QUEUE_ENTITY_CALL       major_push;
	QUEUE_ENTITY_CALL       major_pull;

	volatile long           *minor_have;
	QUEUE_ENTITY_CALL       minor_push;
	QUEUE_ENTITY_CALL       minor_pull;
	
	AO_SpinLockT    w_lock;	/**< 压入锁*/
	AO_SpinLockT    r_lock;	/**< 弹出锁*/
};

bool switch_queue_init(struct switch_queue_info *p_stat,
	unsigned int                            task_size,
	volatile long                           *major_have,
	QUEUE_ENTITY_CALL                       major_push,
	QUEUE_ENTITY_CALL                       major_pull,
	volatile long                           *minor_have,
	QUEUE_ENTITY_CALL                       minor_push,
	QUEUE_ENTITY_CALL                       minor_pull);

// 任务大小比对
// 单独线程读写
bool switch_queue_push(struct switch_queue_info *p_stat, struct supex_task_node *p_node, ...);

bool switch_queue_pull(struct switch_queue_info *p_stat, struct supex_task_node *p_node, ...);

