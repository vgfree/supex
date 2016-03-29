/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef _COMM_CACHE_H_
#define _COMM_CACHE_H_

#include "comm_cache.h"

#define	CACHE_SIZE	1024

static bool _cache_expend(struct comm_cache* comm_cache, int size);

bool commcache_init(struct comm_cache* comm_cache, int capacity)
{
	assert(comm_cache);
	memset(comm_cache, 0, sizeof(struct comm_cache));
	comm_cache->capacity = capacity > 0 ? capacity : CACHE_SIZE;
	comm_cache->cache = calloc(capacity, sizeof(char));
	if( unlikely(!comm_cache->cache) ){
		return false;
	}else{
		return true;
	}
}


void commcache_free(struct comm_cache* comm_cache)
{
	if( likely(comm_cache) ){
		Free(comm_cache->cache);
	}
}

bool commcache_append(struct comm_cache* comm_cache, const char* data, int datasize)
{
	assert(comm_cache);
	int size = datasize > 0 ? datasize : strlen(data);
	if( unlikely(comm_cache->end + datasize > comm_cache->capacity) ){ 
		//缓冲区容量不足， 进行扩容
		bool retval = false;
		retval = _cache_expend(comm_cache, size + comm_cache->end);
		if( unlikely(!retval) ){
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
	comm_cache->start += size;
	comm_cache->size -= size;
	commcache_clean(comm_cache);
}

void commcache_clean(struct comm_cache* comm_cache)
{
	assert(comm_cache);
	if( likely(comm_cache->start != 0) ){
		if( likely( comm_cache->size > 0) ){
			memmove(comm_cache->cache, &comm_cache->cache[comm_cache->start ], comm_cache->size);
		}
		comm_cache->end -= comm_cache->start;
		comm_cache->start = 0;
	}
}

static bool _cache_expend(struct comm_cache* comm_cache, int size)
{
	assert(comm_cache);
	commcache_clean(comm_cache);
	if( likely(size <= comm_cache->capacity) ){
		return true;
	}else{
		char* temp = comm_cache->cache;
		comm_cache->cache = realloc(comm_cache->cache, size);
		if( unlikely(!comm_cache->cache) ){
			comm_cache->cache = temp;
			return false;
		}else{
			comm_cache->capacity = size;
			return true;
		}
	}
}

