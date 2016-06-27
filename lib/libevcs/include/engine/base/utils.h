#pragma once

// #include "config.h"

/* Obtain O_* constant definitions */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <mqueue.h>
#include <ctype.h>

#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/shm.h>
#include <sched.h>
#include <openssl/sha.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <libmini.h>

#include "spx_common.h"
#include "errors.h"

/*
 *  if dst = NULL, then free dst
 */
char *sha1_hash(const void *src, unsigned long len, char *dst);

union virtual_system
{
	void            *base;
	lua_State       *L;	/*if use lua language*/
};

#define GET_NEED_COUNT(idx, div) ((idx) / (div) + (((idx) % (div) >= 1) ? 1 : 0))

/*==============================================================================================*
*       POSIX锁定义                                    *
*==============================================================================================*/
#ifdef USE_MUTEX
  #define X_NEW_LOCK pthread_mutex_t
  #define X_LOCK_INIT(lock)     pthread_mutex_init((lock), NULL)
  #define X_LOCK(lock)          pthread_mutex_lock((lock))
  #define X_TRYLOCK(lock)       pthread_mutex_trylock((lock))
  #define X_UNLOCK(lock)        pthread_mutex_unlock((lock))
#else
  #define X_NEW_LOCK pthread_spinlock_t
  #define X_LOCK_INIT(lock)     pthread_spin_init((lock), 0)
  #define X_LOCK(lock)          pthread_spin_lock((lock))
  #define X_TRYLOCK(lock)       pthread_spin_trylock((lock))
  #define X_UNLOCK(lock)        pthread_spin_unlock((lock))
#endif

/*==============================================================================================*
*       初始化操作                                   *
*==============================================================================================*/

/* 安全初始化 */
struct safe_once_init
{
	AO_T            magic;	/**< 魔数*/
	AO_SpinLockT    lock;
};

/* 安全初始化开始 */
#define SAFE_ONCE_INIT_COME(safe)			     \
	STMT_BEGIN					     \
	AO_SpinLock(&(safe)->lock);			     \
	STMT_BEGIN					     \
	if (AO_SWAP(&(safe)->magic, OBJMAGIC) == OBJMAGIC) { \
		break;					     \
	}

/* 安全初始化结束 */
#define SAFE_ONCE_INIT_OVER(safe)     \
	STMT_END;		      \
	AO_SpinUnlock(&(safe)->lock); \
	STMT_END

/*
 * 日期/时间格式定义
 */

/*
 * 打印时间信息
 */
#define x_printtime(x)					 \
	STMT_BEGIN					 \
	gettimeofday(((struct timeval *)x), NULL);	 \
	x_printf(D, "time: %ld.%ld s.",			 \
		(long)(((struct timeval *)x)->tv_sec),	 \
		(long)(((struct timeval *)x)->tv_usec)); \
	STMT_END

int get_overplus_time(void);

int get_current_time(void);

/*==============================================================================================*
*       fifo function                                   *
*==============================================================================================*/
#define FIFO_DATA       "-DATA"
#define FIFO_COMD       "-COMD"

void fifo_init(int size);

char *get_fifo_msg(void);

char *put_fifo_msg(char *data, int size);

/*==============================================================================================*
*       msmq function                                   *
*==============================================================================================*/
// #define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/*POSIX事实信号*/
#ifndef SIGRTMIN
  #define SIGRTMIN      34
#endif

#define APP_SIGNAL      (SIGRTMIN + 1)

enum
{
	MSMQ_LEVEL_PRIOLOW = 1,
	MSMQ_LEVEL_PRIOHIGH,
};
typedef void (*SHELL_CNTL_CB)(const char *data);

#define MSMQ_MAX_DATA_SIZE 512
struct msg_info	// TODO
{ char  opt;
  char  mode;
  int   time;
  char  data[MSMQ_MAX_DATA_SIZE]; };

void msmq_init(char *name, SHELL_CNTL_CB func);

int msmq_hand(void);

int msmq_call(void);

void msmq_exit(void);

int msmq_send(char *name, char *data, size_t size);

/*==============================================================================================*
*       pthread function                                    *
*==============================================================================================*/
struct safe_init_base
{
	int             count;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
};

struct safe_init_step
{
	struct safe_init_base   *base;
	int                     step;
	void                    *addr;
	void                    *data;
};
#ifndef NOT_BIND_CPU
  #define SAFE_PTHREAD_INIT_COME(step) \
	STMT_BEGIN		       \
	BindCPUCore(-1);	       \
	pthread_mutex_lock(&(step)->base->lock)
#else
  #define SAFE_PTHREAD_INIT_COME(step) \
	STMT_BEGIN		       \
	pthread_mutex_lock(&(step)->base->lock)
#endif

#define SAFE_PTHREAD_INIT_OVER(step)		   \
	(step)->base->count++;			   \
	pthread_mutex_unlock(&(step)->base->lock); \
	pthread_cond_signal(&(step)->base->cond);  \
	STMT_END

void safe_start_pthread(void *func, int num, void *addr, void *data);

/*
 * 给lua的分配函数
 */
static inline
void *lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud;
	(void)osize;

	if (nsize == 0) {
		free(ptr);
		return NULL;
	} else {
		return realloc(ptr, nsize);
	}
}

int mkdir_if_doesnt_exist(const char *dirpath);

