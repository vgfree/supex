/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_cache.h"

#define INCREASE_SIZE	1024


void commcache_init(struct comm_cache* commcache)
{
	assert(commcache);
	memset(commcache, 0, sizeof(struct comm_cache));
	commcache->capacity = BASEBUFFERSIZE;
	commcache->buffer = commcache->base; 
	commcache->init = true;
	return ;
}

void commcache_free(struct comm_cache* commcache)
{
	if (commcache && commcache->init && commcache->buffer != commcache->base) {
		Free(commcache->buffer);
		commcache->init = false;
	}
	return ;
}

bool commcache_append(struct comm_cache* commcache, const char* data, int datasize)
{
	assert(commcache && commcache->init);
	int size = datasize > 0 ? datasize : strlen(data);
	datasize =  commcache->end + size - commcache->capacity;
	if (unlikely(datasize > 0)) { 
		//缓冲区容量不足， 进行扩容
		if (unlikely(!commcache_expend(commcache, datasize))) {
			return false;
		}
	}
	
	memcpy(&commcache->buffer[commcache->end ], data, size);
	commcache->end += size;
	commcache->size += size;
	return true;
}

void commcache_clean(struct comm_cache* commcache)
{
	assert(commcache && commcache->init);
	if (likely(commcache->start != 0)) {
		if (likely( commcache->size > 0)) {
			memmove(commcache->buffer, &commcache->buffer[commcache->start ], commcache->size);
		}
		commcache->end -= commcache->start;
		commcache->start = 0;
	}
}

bool commcache_expend(struct comm_cache* commcache, int size)
{
	assert(commcache && commcache->init);
	int	length	= commcache->size;
	char*	buffer	= commcache->buffer;
	int	capacity= commcache->capacity + ((size > 0) ? size : INCREASE_SIZE);

	commcache_clean(commcache);

	NewArray(commcache->buffer, capacity);	
	if (commcache->buffer) {
		commcache->capacity = capacity;
		memcpy(commcache->buffer, buffer, length);
		if (buffer != commcache->base) {
			Free(buffer);
		}
		return true;
	} else {
		commcache->buffer = buffer;
		return false;
	}
}

void commcache_restore(struct comm_cache* commcache)
{
	assert(commcache && commcache->init);
	if (commcache->buffer != commcache->base) {
		if (commcache->size < BASEBUFFERSIZE) {
			if (commcache->size > 0) {
				memcpy(commcache->base, &commcache->buffer[commcache->start], commcache->size);
			}
			Free(commcache->buffer);
			commcache->buffer = commcache->base;
			commcache->capacity = BASEBUFFERSIZE;
			printf("restore cache capacity:%d\n", commcache->capacity);
		}
	}
}
