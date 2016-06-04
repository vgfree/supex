//
//  network_connect_pool.c
//  supex
//
//  Created by 周凯 on 15/9/19.
//  Copyright © 2015年 zk. All rights reserved.
//
#include <zmq.h>
#include "connection_pool.h"

#define _CONNECTION_POOL_SIZE   5000
#define _CONNECT_POOLW_SIZE     5000
#define _CONNECT_POOLS_SIZE     500

/*连接池*/
static struct connect_pool
{
	int     magic;
	SListT  pools;
} *_g_cntpool = NULL;

#ifndef __GNUC__
  #error "need gcc compiler."
#endif

static void _connect_pool_init() __attribute__((constructor()));

static int _find_connection(const char *buff, int size, void *usr, int len);

/*
 * 在未决连接池中查找是否有存在的连接
 */
bool add_connection(struct iohandle *io)
{
	const char      *data = NULL;
	bool            rc = false;

	assert(io);

	SListLock(_g_cntpool->pools);

	data = SListLookup(_g_cntpool->pools, _find_connection, (void *)io->name, 0, NULL);

	if (unlikely(data)) {
		rc = false;
	} else {
		int effect = 0;
		rc = SListAddHead(_g_cntpool->pools, (char *)&io, sizeof(io), &effect, NULL);

		if (unlikely(rc && (effect != sizeof(io)))) {
			rc = false;
		}
	}

	SListUnlock(_g_cntpool->pools);
	return rc;
}

/*
 * 删除所有失效的连接
 */
void delete_connection(struct iohandle *io)
{
	const char *data = NULL;

	assert(io);

	SListLock(_g_cntpool->pools);

	data = SListLookup(_g_cntpool->pools, _find_connection, (void *)io->name, 0, NULL);

	if (likely(data)) {
		SListDelData(_g_cntpool->pools, data);
	}

	SListUnlock(_g_cntpool->pools);
}

/*
 * 销毁所有链接池中的连接
 */
void destroy_connection_pool()
{
	return_if_fail(UNREFOBJ(_g_cntpool));

	Free(_g_cntpool->pools);
	Free(_g_cntpool);
}

static void _connect_pool_init()
{
	long                            size = 0;
	char *volatile                  buff = NULL;
	struct connect_pool *volatile   pool = NULL;

	return_if_fail(!_g_cntpool);

	TRY
	{
		New(pool);
		AssertError(pool, ENOMEM);

		size = SListCalculateSize(_CONNECTION_POOL_SIZE, sizeof(uintptr_t));

		NewArray0(size, buff);
		AssertError(buff, ENOMEM);

		pool->pools = SListInit((char *)buff, size, sizeof(uintptr_t), false);
		ASSERTOBJ(pool->pools);

		REFOBJ(pool);

		if (unlikely(!AO_CASB(&_g_cntpool, NULL, (void *)pool))) {
			RAISE(EXCEPT_ASSERT);
		}
	}
	CATCH
	{
		Free(buff);
		Free(pool);
		RERAISE;
	}
	END;
}

static int _find_connection(const char *buff, int size, void *usr, int len)
{
	struct iohandle *io = *(struct iohandle **)buff;
	int             flag = 0;

	assert(usr);
	//	ASSERTOBJ(hdl);

	flag = strcmp(usr, io->name);
	return unlikely(flag == 0) ? 0 : -1;
}

