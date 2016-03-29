//
//  oper_task_queue.c
//  supex
//
//  Created by 周凯 on 15/9/12.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "oper_task_queue.h"

struct ext_tqueue g_ext_tqueue = {};

/*文件队列方法*/
static void *ext_tfilequeue_create(struct ext_tqueue *etqueue, void *name)
{
	FileQueueT      fqueue = NULL;
	char            fpath[MAX_PATH_SIZE] = {};

	ASSERTOBJ(etqueue);

	assert(name);

	ASSERTOBJ(fqueue);

	return fqueue;
}

/*共享内存方法*/
static void *ext_tshmqueue_create(struct ext_tqueue *etqueue, void *name)
{
	ShmQueueT       squeue = NULL;
	intptr_t        iname = (intptr_t)name;

	barrier();

	ASSERTOBJ(etqueue);

	if (unlikely((iname > 0xfffffL) || (iname < 0L))) {
		x_printf(W, "the hint key of shared memory is out of range, "
			"its range is 0~0xff, but it gots is `%p`.", name);
		iname = 0xffffff;
	}

	iname |= etqueue->namehint;

	squeue = SHM_QueueCreate((key_t)iname, etqueue->qtotalnodes, etqueue->qnodesize);

	ASSERTOBJ(squeue);

	return squeue;
}

bool ext_tqueue_init(struct ext_tqueue *tqueue, const char *type, const char *hint, const char *path)
{
	assert(tqueue && type && hint);

	if (unlikely(ISOBJ(tqueue))) {
		x_printf(W, "it has been initialize yet.");
		return true;
	}

	if (strcasecmp(type, "shm") == 0) {
		tqueue->type = ETT_SHMQ;
	} else if (strcasecmp(type, "file") == 0) {
		tqueue->type = ETT_FILEQ;
	} else {
		x_printf(E, "unknow type.");
		return false;
	}

	switch (tqueue->type)
	{
		case ETT_SHMQ:
		{
			intptr_t ihint = 0;

			ihint = atoi(hint);

			if (unlikely((ihint > 0xffL) || (ihint < 0L))) {
				x_printf(E, "the hint key of shared memory is out of range, "
					"its range is 0~0xff, but it gots is `%s`.", hint);
				return false;
			}

			tqueue->namehint = ihint << 24;
			tqueue->createq = ext_tshmqueue_create;
			break;
		}

		case ETT_FILEQ:
		{
			if (unlikely(!path)) {
				tqueue->path = x_strdup("./");
			} else {
				struct stat     pstat = {};
				int             flag = 0;

				flag = stat(path, &pstat);

				if (unlikely(flag < 0)) {
					x_printf(E, "check the directory `%s` failed : %s.",
						path, x_strerror(errno));
					return false;
				}

				if (unlikely(!S_ISDIR(pstat.st_mode))) {
					x_printf(E, "expect `%s` is a directory.", path);
					return false;
				}

				flag = access(path, R_OK | W_OK | X_OK);

				if (unlikely(flag < 0)) {
					x_printf(E, "checks the accessibility of the directory "
						"`%s` failed : %s.", path, x_strerror(errno));
					return false;
				}

				tqueue->path = x_strdup(path);
			}

			break;
		}

		default:
			x_printf(E, "unknow type.");
			return false;
	}

	return true;
}

