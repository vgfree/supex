/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_cache.h"

#define	CACHE_SIZE		1024
#define CACHE_INCREASE_SIZE	1024


bool commcache_init(struct comm_cache* comm_cache, int capacity)
{
	assert(comm_cache);
	memset(comm_cache, 0, sizeof(struct comm_cache));
	comm_cache->capacity = capacity > 0 ? capacity : CACHE_SIZE;
	comm_cache->cache = calloc(capacity, sizeof(char));
	if (unlikely(!comm_cache->cache)) {
		return false;
	} else {
		comm_cache->init = true;
		return true;
	}
}


void commcache_free(struct comm_cache* comm_cache)
{
	if (likely(comm_cache && comm_cache->init)) {
		Free(comm_cache->cache);
	}
}

bool commcache_append(struct comm_cache* comm_cache, const char* data, int datasize)
{
	assert(comm_cache && comm_cache->init);
	int size = datasize > 0 ? datasize : strlen(data);
	if (unlikely(comm_cache->end + datasize > comm_cache->capacity)) { 
		//缓冲区容量不足， 进行扩容
		bool retval = false;
		retval = commcache_expend(comm_cache, size);
		if (unlikely(!retval)) {
			return false;
		}
	}
	
	memcpy(&comm_cache->cache[comm_cache->end ], data, size);
	comm_cache->end += size;
	comm_cache->size += size;
	return true;
}

void commcache_deccnt(struct comm_cache* comm_cache, int size)
{
	assert(comm_cache && comm_cache->init);
	comm_cache->start += size;
	comm_cache->size -= size;
	commcache_clean(comm_cache);
}

void commcache_clean(struct comm_cache* comm_cache)
{
	assert(comm_cache && comm_cache->init);
	if (likely(comm_cache->start != 0)) {
		if (likely( comm_cache->size > 0)) {
			memmove(comm_cache->cache, &comm_cache->cache[comm_cache->start ], comm_cache->size);
		}
		comm_cache->end -= comm_cache->start;
		comm_cache->start = 0;
	}
}

bool commcache_expend(struct comm_cache* comm_cache, int size)
{
	assert(comm_cache && comm_cache->init);
	commcache_clean(comm_cache);
	if (size <= 0) {
		size = CACHE_INCREASE_SIZE;
	}
	char *temp = comm_cache->cache;
	
	comm_cache->cache = realloc(comm_cache->cache, comm_cache->end + size);
	if (unlikely(!comm_cache->cache)) {
		comm_cache->cache = temp;
		return false;
	} else {
		comm_cache->capacity = size;
		return true;
	}
}
