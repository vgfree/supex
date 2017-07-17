/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/14.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_cache.h"

#define INCREASE_SIZE 1024

void commcache_init(struct comm_cache *commcache)
{
	assert(commcache);
	memset(commcache, 0, sizeof(struct comm_cache));
	commcache->capacity = BASEBUFFERSIZE;
	commcache->buffer = commcache->base;
	commcache->init = true;
}

void commcache_free(struct comm_cache *commcache)
{
	if (commcache && commcache->init && (commcache->buffer != commcache->base)) {
		Free(commcache->buffer);
		commcache->init = false;
	}
}

bool commcache_append(struct comm_cache *commcache, const char *data, int datasize)
{
	assert(commcache && commcache->init);
	int size = datasize > 0 ? datasize : strlen(data);
	datasize = commcache->end + size - commcache->capacity;

	if (unlikely(datasize > 0)) {
		// 缓冲区容量不足， 进行扩容
		if (unlikely(!commcache_expand(commcache, datasize))) {
			return false;
		}
	}

	memcpy(&commcache->buffer[commcache->end], data, size);
	commcache->end += size;
	commcache->size += size;
	return true;
}

void commcache_adjust(struct comm_cache *commcache)
{
	assert(commcache && commcache->init);

	if (likely(commcache->start != 0)) {
		if (likely(commcache->size > 0)) {
			memmove(commcache->buffer, &commcache->buffer[commcache->start], commcache->size);
		}

		commcache->end -= commcache->start;
		commcache->start = 0;
	}
}

bool commcache_expand(struct comm_cache *commcache, int size)
{
	assert(commcache && commcache->init);
	int     length = commcache->size;
	char    *buffer = commcache->buffer;
	int     remain = commcache->capacity - length;

	commcache_adjust(commcache);

	if (remain >= size) {
		return true;
	}

	int     update = size - remain;
	int     capacity = commcache->capacity + ((update > INCREASE_SIZE) ? update : INCREASE_SIZE);

	if (buffer == commcache->base) {
		/*基地址不用realloc*/
		NewArray(commcache->buffer, capacity);
	} else {
		commcache->buffer = realloc(buffer, capacity);
	}

	if (commcache->buffer) {
		commcache->capacity = capacity;

		if (buffer == commcache->base) {
			memcpy(commcache->buffer, buffer, length);
		}

		return true;
	} else {
		commcache->buffer = buffer;
		return false;
	}
}

void commcache_shrink(struct comm_cache *commcache)
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
			commcache->end -= commcache->start;
			commcache->start = 0;
			printf("restore cache capacity:%d\n", commcache->capacity);
			return;
		}
	}
	commcache_adjust(commcache);
}

