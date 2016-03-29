/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef _COMM_UTILS_H_
#define _COMM_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


/* 逻辑跳转优化*/
#if GCC_VERSION
/*条件大多数为真，与if配合使用，直接执行if中语句*/
  #define likely(x)     __builtin_expect(!!(x), 1)
/*条件大多数为假，与if配合使用，直接执行else中语句*/
  #define unlikely(x)   __builtin_expect(!!(x), 0)
#else
  #define likely(x)     (!!(x))
  #define unlikely(x)   (!!(x))
#endif


#undef	New
#define New(ptr)						\
	((ptr) = __typeof__(ptr)calloc(1, sizeof(*(ptr))),	\
	(likely((ptr)) ? ptr :(errno = ENOMEM, NULL)))

#undef	Free
#define Free(ptr)						\
	 do{						\
		if( likely((ptr))){				\
			free((void*)(ptr));			\
			(ptr) = (__typeof__(ptr)) 0;		\
		}						\
	 }while(0)

	
/*设置指定描述符的标志*/
static inline bool set_fdopt(int fd, int flag)
{
	int retval = -1;
	retval = fcntl(fd, F_GETFL, NULL);
	if( unlikely(retval == -1) ){
		return false;
	}
	retval |= flag;
	retval = fcntl(fd, F_SETFL, retval);
	if( unlikely(retval == -1) ){
		return false;
	}
	return true;
}

#endif

