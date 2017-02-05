//
//  _evcoro_common.h
//  libmini
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef _evcoro_common_h
#define _evcoro_common_h

#ifndef AO_T
typedef volatile long AO_T;
#endif
/* ------------------------------------------------------ */
#ifndef likely
  #if defined __GNUC__
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
  #else
    #define likely(x)   (!!(x))
    #define unlikely(x) (!!(x))
  #endif
#endif

#if !defined(__LINUX__) && (defined(__linux__) || defined(__KERNEL__) \
	|| defined(_LINUX) || defined(LINUX) || defined(__linux))
  #define  __LINUX__    (1)
#elif !defined(__APPLE__) && defined(__MacOSX__)
  #define  __APPLE__    (1)
#elif !defined(__CYGWIN__) && (defined(__CYGWIN32__) || defined(CYGWIN))
  #define  __CYGWIN__   (1)
#endif


#ifdef __LINUX__
#include <sys/eventfd.h>
#else
#define EFD_SEMAPHORE	(1)
#define EFD_NONBLOCK	(04000)
#define eventfd_t	uint64_t
static inline int eventfd_write(int fd, eventfd_t value)
{
	return write(fd, &value, sizeof(eventfd_t)) !=
			sizeof(eventfd_t) ? -1 : 0;
}

static inline int eventfd_read(int fd, eventfd_t *value)
{
	return read(fd, value, sizeof(eventfd_t)) !=
			sizeof(eventfd_t) ? -1 : 0;
}

static inline int eventfd(unsigned int initval, int flags)
{
	return syscall(__NR_eventfd2, initval, flags);
}
#endif

#endif	/* _evcoro_common_h */
