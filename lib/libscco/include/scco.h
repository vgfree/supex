#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <ucontext.h>

#ifndef SCCO_USE_STATIC_STACK
  #define SCCO_USE_LEAST_SPACE
#endif
#define OPEN_FAST_SWITCH
#define OPEN_LESS_SWITCH

#define SCCO_SAFE_SIGNATURE     0x6a

#ifdef SCCO_USE_LEAST_SPACE
  #define STACK_SIZE            (1024 * 1024 * 16)
  #define SCCO_QUEUE_ADD_STEP   32
#else
  #define STACK_SIZE            (1024 * 1024 * 8)
  #define SCCO_QUEUE_ADD_STEP   2
#endif

#define SCCO_QUEUE_LOW_SLOT     (SCCO_QUEUE_ADD_STEP * 15)
#define SCCO_QUEUE_MIN_SLOT     (SCCO_QUEUE_ADD_STEP * 16)

#ifdef OPEN_FAST_SWITCH
  #define OPEN_MAIN_STATUS_CNTL
#endif

struct schedule;
typedef void *(*coroutine_func)(struct schedule *S, void *data);

/*
 * safe_mark must add at head and tail,because stack is grow upwards.
 * Of course, we can also add safe_mark at tail and move stack to head.
 */
/*协同成员*/
struct coroutine
{
#ifdef SCCO_USE_LEAST_SPACE
	char                    *stack;			/**< dynamic stack space*/
	ptrdiff_t               maxsize;		/**< stack max size*/
	ptrdiff_t               offsize;		/**< stack use size*/
#else
	char                    stack[STACK_SIZE];	/**< static stack space*/
#endif
	struct schedule         *sch;			/**< 所属调度器*/
	ucontext_t              ctx;			/**< 运行上下文*/

	int                     status;			/**< 上下文状态*/
	int                     origin;			/**< 数据空间位于栈/堆*/

	coroutine_func          func;			/**< 协同回调*/
	void                    *data;			/**< 回调参数*/

	struct coroutine        *prev;			/**< 上一个切换的协同*/
	struct coroutine        *next;			/**< 下一个切换的协同*/
#ifdef SCCO_USE_LEAST_SPACE
#else
	char                    safe_mark;
#endif
};

/*调度器*/
struct schedule
{
#ifdef SCCO_USE_LEAST_SPACE
	char                    stack[STACK_SIZE];
#endif
	int                     nubs;	/*coroutine count*/
#ifdef OPEN_LESS_SWITCH
	int                     last;	/*last coroutine count*/
	bool                    append;
#endif
	int                     origin;	/**< 数据空间来源*/
	struct coroutine        main;	/**< 主协同*/
	struct coroutine        *exec;	/*exec context*/
	struct coroutine        *exit;	/*exit context*/

	void                    (*_switch_)(struct schedule *S);
};

struct schedule *coroutine_open(struct schedule *S, coroutine_func func, void *data);

void coroutine_close(struct schedule *S);

/*===dynamic method===*/
struct coroutine        *coroutine_create(struct schedule *S, coroutine_func func, void *data);

/*=======ending=======*/

/*===static method===*/
int coroutine_init(struct schedule *S, struct coroutine *C, coroutine_func func, void *data);

/**
 * 重置协程状态为初始化状态
 * 成功返回0，失败则返回-1
 */
int coroutine_reuse(struct coroutine *C);

/*=======ending=======*/

/**
 * 指定协程时候状态可用
 * 可用则返回0，否则返回-1
 */
int coroutine_useable(struct coroutine *C);

void coroutine_switch(struct schedule *S);

void coroutine_loop(struct schedule *S);

