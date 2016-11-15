/**
 * @file   buffer_erase.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Jan 21 18:27:58 2010
 *
 * @brief  buffer_erase unit test
 *
 *
 */

#include "rinoo/rinoo.h"

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_buffer *buffer;

	buffer = buffer_create(NULL);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == RINOO_BUFFER_HELPER_INISIZE);
	XTEST(buffer->class != NULL);
	XTEST(buffer_add(buffer, "bl1bl1bl1", 9) == 9);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "bl1bl1bl1", 9) == 0);
	XTEST(buffer->size == 9);
	XTEST(buffer->msize == RINOO_BUFFER_HELPER_INISIZE);
	XTEST(buffer_add(buffer, "bl2bl2bl2", 9) == 9);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "bl1bl1bl1bl2bl2bl2", 18) == 0);
	XTEST(buffer->size == 18);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 18));
	XTEST(buffer_erase(buffer, 9) == 0);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "bl2bl2bl2", 9) == 0);
	XTEST(buffer->size == 9);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 18));
	XTEST(buffer_erase(buffer, 9) == 0);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 18));
	XTEST(buffer_add(buffer, "bl2bl2bl2", 9) == 9);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "bl2bl2bl2", 9) == 0);
	XTEST(buffer->size == 9);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 18));
	XTEST(buffer_erase(buffer, 0) == 0);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 18));
	buffer_destroy(buffer);
	XPASS();
}
