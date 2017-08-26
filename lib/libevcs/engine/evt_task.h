#pragma once

#include "base/utils.h"

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


/*
 * Return 0 on success, or -1 if efd has been made nonblocking and
 * errno is EAGAIN.  If efd has been marked blocking or the eventfd counter is
 * not zero, this function doesn't return error.
 */
int eventfd_xrecv(int efd, eventfd_t *value);
int eventfd_xsend(int efd, eventfd_t value);
int eventfd_xwait(int efd, int timeout);



struct evt_task {
	int efd;
};

struct evt_task *evt_task_init(void);
void evt_task_free(struct evt_task *etask);
void evt_task_awake(struct evt_task *etask);
void evt_task_sleep(struct evt_task *etask);
bool evt_task_twait(struct evt_task *etask, int msec);
