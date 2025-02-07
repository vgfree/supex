//
//  _ev_coro_task.h
//  libmini
//
//  Created by 周凯 on 15/12/1.
//  Copyright © 2015年 zk. All rights reserved.
//
#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "evcoro_scheduler.h"
#if __cplusplus
extern "C" {
#endif

//#define USE_LIBCORO

#ifdef USE_LIBCORO
#include "coro.h"
#else
#include <ucontext.h>
	typedef struct coro_context coro_context;

	struct coro_context
	{
		ucontext_t uc;
	};

	struct coro_stack
	{
		void    *sptr;
		size_t  ssze;
	};

#define coro_transfer(p, n)   swapcontext(&((p)->uc), &((n)->uc))
#define coro_destroy(ctx)     (void )(ctx)

	static inline
		void coro_stack_free(struct coro_stack *stack)
		{
			if (stack->sptr) {
				//printf("free stack->sptr %p\n", stack->sptr);
				free(stack->sptr);
				stack->sptr = NULL;
				stack->ssze = 0;
			}
		}

	static inline
		int coro_stack_alloc(struct coro_stack *stack, unsigned int size)
		{
			void    *base = calloc(1, size);
			if (unlikely(!base)) {
				return 0;
			}

			//printf("new stack->sptr %p\n", base);
			stack->sptr = base;
			stack->ssze = size;
			return 1;
		}
#endif

/*
 * 缺陷:此功能函数操作的协程对象不能移交给其它线程。
 * 因为:当线程a执行回调函数fcb把协程移交给线程b,
 * 	a可能让出cpu暂时未执行setcontext,
 * 	b先b开始使用协程或同时使用都会出现异常.
 */
#define coro_saveswap(p, n, f, d, ...)   do {      \
	bool (*fcb)(void *, ...) = (bool (*)(void *, ...))f;   \
	volatile int once = 0;   \
	getcontext(&((p)->uc));	\
	/* printf("coro_saveswap---- %d \n", once); */	\
	if (0 == once++) {      \
		if (fcb) { \
			/* printf("do fcb ... .. .\n");	*/	\
			(*fcb)(d, ##__VA_ARGS__);        \
		}       \
		/* printf("test coredump point 1\n"); sleep(2); printf("test coredump point 2\n"); */ \
		setcontext(&((n)->uc));	\
	}	\
} while (0);



/* ------------------------------------------------------ *\
* 协程对象定义
* 所有协程要不在一个循环链表里管理，要不就是在一个栈链表中。
* 协程对象有一个协程上下文，函数在这个上下文上运行。
* 运行中的协程对象有从属的协程上下文，当协程对象消耗时，会将控制交还给它的
* 的主协程。
* 协程在运行状态指示该协程是否应该继续被运行，休眠或重用。
* 每个协程必定从属于一个调度器
\* ------------------------------------------------------ */
struct ev_coro
{
	struct ev_coro          *prev;	/*上一个协程对象，如果无则为null*/
	struct ev_coro          *next;	/*下一个协程对象，如果无则为null*/

	coro_context            ctx;	/*协程运行上下文*/
	coro_context            *main;	/*如果是主协程上下文，则为nullptr*/

	AO_T                     depth;	/*调度状态切换的深度*/

	int			slinks; /*引用计数*/
	evcoro_taskcb           call;	/*协程运行时的入口函数*/
	void                    *usr;	/*协程运行时的入口函数的参数*/
	enum
	{
		EVCORO_STAT_INITING = 0,		/*初始化状态，创建或刚装载到调度器时*/
		EVCORO_STAT_RUNNING,		/*被调度，正常运行状态*/
		EVCORO_STAT_SUSPENDING,		/*被调度，但挂起状态*/
		EVCORO_STAT_EXITED,		/*被调度，但已从任务函数中正常退出*/
		EVCORO_STAT_TOSWAP,		/*被调度，需要被移交给其它调度器*/
		//EVCORO_STAT_EXCEPT,		/*被调度，但发生了异常，需要销毁*/
		//EVCORO_STAT_TIMEDOUT,		/*被调度，当发生了运行超时*/
	}                       stat;		/*协程的状态*/

	struct coro_stack       stack;		/*运行栈空间*/
	ssize_t                 stss;		/*运行栈空间大小*/
	struct evcoro_scheduler *scheduler;	/*所属调度器*/

	struct ev_coroep
	{
		evcoro_destroycb        destroy;	/*清理函数*/
		void                    *usr;		/*清理函数参数*/
		struct ev_coroep        *next;		/*下一个清理函数节点*/
	}                       *cleanup;		/*清理函数对象链表*/
};
/* ------------------------------------------------------ */

/**
 * 创建一个协程对象
 * @param scheduler 所属调度器（包含协程上下文）
 * @param call 任务函数入口
 * @param ss 运行栈大笑
 * @return 返回对象
 */
struct ev_coro  *_evcoro_create(struct evcoro_scheduler *scheduler,
	evcoro_taskcb call, void *usr, size_t ss);

/**
 * 销毁协程对象
 * @param coro
 * @param cache 是否将当前将要被销毁对象缓存
 */
void _evcoro_destroy(struct ev_coro *coro, bool cache);

/*是否是子协程对象*/
#define _evcoro_subctx(ptr)             (likely((ptr)->main != NULL))
/*是否是主协程对象（调度器的协程上下文）*/
#define _evcoro_mainctx(ptr)            (unlikely((ptr)->main == NULL))

/*是否是初始化状态*/
#define _evcoro_stat_initing(ptr)          (likely((ptr)->stat == EVCORO_STAT_INITING))
/*是否是运行状态*/
#define _evcoro_stat_running(ptr)       (likely((ptr)->stat == EVCORO_STAT_RUNNING))
/*是否是挂起状态*/
#define _evcoro_stat_suspending(ptr)    (likely((ptr)->stat == EVCORO_STAT_SUSPENDING))
/*是否是正常退出状态*/
#define _evcoro_stat_exited(ptr)        (unlikely((ptr)->stat == EVCORO_STAT_EXITED))
/*是否是需要移交状态*/
#define _evcoro_stat_toswap(ptr)        (unlikely((ptr)->stat == EVCORO_STAT_TOSWAP))

/**
 * 将coro节点插入到cursor的上一个节点与cursor之间
 * @param cursor 光标节点，已在链表中
 * @param coro 加入节点，未在链表中
 */
static inline
void _evcoro_push(struct ev_coro *cursor, struct ev_coro *coro)
{
	coro->next = cursor;
	coro->prev = cursor->prev;
	cursor->prev->next = coro;
	cursor->prev = coro;
}

/**
 * 将coro节点从所在到链中移除
 */
static inline
void _evcoro_pull(struct ev_coro *coro)
{
	coro->prev->next = coro->next;
	coro->next->prev = coro->prev;
}

/**
 * 推入一个清理函数回调
 */
static inline
bool _evcoro_cleanup_push(struct ev_coro *coro, evcoro_destroycb destroy, void *usr)
{
	struct ev_coroep *ep = calloc(1, sizeof(*ep));

	if (unlikely(!ep)) {
		return false;
	}

	ep->next = coro->cleanup;
	ep->usr = usr;
	ep->destroy = destroy;
	coro->cleanup = ep;
	return true;
}

/**
 * 弹出一个清理回调，并执行
 * @param execute 如果为false则不执行回调
 */
static inline
bool _evcoro_cleanup_pop(struct ev_coro *coro, bool execute)
{
	if (unlikely(!coro->cleanup)) {
		return false;
	}

	struct ev_coroep *ep = coro->cleanup;
	coro->cleanup = ep->next;

	if (execute) {
		ep->destroy(ep->usr);
	}

	free(ep);
	return true;
}

/* ------------------------------------------------------ *\
* 协程管理对象，其可以被定义成循环链表或栈
* 协程管理对象的头节点协程指针指向调度器所在的协程（线程）上下文
* 栈管理没有主协程
* 通过其中的光标节点指向当前运行上下文
\* ------------------------------------------------------ */
struct ev_coro_t
{
	struct ev_coro  *head;		/*头节点，栈结构中为null*/
	struct ev_coro  *cursor;	/*光标节点*/
	unsigned        cnt;		/*节点数量，不包括头节点*/
};

/**
 * 创建环形链表管理器，主要用于调度器循环调度可以运行的协程或暂存暂停的协程对象
 */
struct ev_coro_t        *evcoro_list_new(struct evcoro_scheduler *scheduler);

/**
 * 销毁环形管理器，如果destroy不为null，则用于销毁协程对象的入口函数的参数（用户数据）
 */
void evcoro_list_destroy(struct ev_coro_t *list, evcoro_destroycb destroy);

/**
 * 创建栈管理器，主要用于调度器暂存停止但可重用的协程对象
 */
struct ev_coro_t        *evcoro_stack_new(void);

/**
 * 销毁栈管理器
 */
void evcoro_stack_destroy(struct ev_coro_t *stack);

/*当前运行协程*/
#define evcoro_list_cursor(ptr)         ((ptr)->cursor)
/*当前主协程*/
#define evcoro_list_head(ptr)           ((ptr)->head)
/*移动当前运行协程到下一个，移动后需要切换才能在其上面运行*/
#define evcoro_list_movenext(ptr)       ((ptr)->cursor = (ptr)->cursor->next)
/*移动当前运行协程到上一个，移动后需要切换才能在其上面运行*/
#define evcoro_list_moveprev(ptr)       ((ptr)->cursor = (ptr)->cursor->prev)
/*管理器中协程数量*/
#define evcoro_list_counter(ptr)        ((ptr)->cnt)

/**
 * 加入协程对象到链中，插入到list的cursor节点与cursor上一个节点之间
 */
static inline
void evcoro_list_push(struct ev_coro_t *list, struct ev_coro *coro)
{
	_evcoro_push(evcoro_list_cursor(list), coro);
	evcoro_list_counter(list)++;
}

/**
 * 将光标移动到上一个协程对象，并从链中移除先前的光标协程对象
 */
static inline
struct ev_coro *evcoro_list_pull(struct ev_coro_t *list)
{
	struct ev_coro *coro = evcoro_list_cursor(list);

	evcoro_list_moveprev(list);
	evcoro_list_counter(list)--;
	_evcoro_pull(coro);
	return coro;
}

/* ------------------------------------------------------ */
#if __cplusplus
}
#endif
