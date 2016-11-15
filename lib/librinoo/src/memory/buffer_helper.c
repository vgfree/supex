/**
 * @file   buffer_helper.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Oct 21 14:22:10 2012
 *
 * @brief  Buffer helper functions
 *
 *
 */

#include "rinoo/memory/module.h"

size_t buffer_helper_growthsize(t_buffer *buffer, size_t newsize)
{
	if (newsize >= buffer->class->maxsize) {
		return buffer->class->maxsize;
	}
	if (newsize < buffer->msize) {
		return buffer->msize;
	}
	return (size_t)(newsize * 1.5);
}

void *buffer_helper_malloc(t_buffer *unused(buffer), size_t size)
{
	return malloc(size);
}

void *buffer_helper_realloc(t_buffer *buffer, size_t newsize)
{
	return realloc(buffer->ptr, newsize);
}

int buffer_helper_free(t_buffer *buffer)
{
	free(buffer->ptr);
	buffer->ptr = NULL;
	return 0;
}
