/**
 * @file   buffer_extend.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Jan 21 18:27:58 2010
 *
 * @brief  buffer_extend unit test
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
	XTEST(buffer_extend(buffer, 0) == 0);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == RINOO_BUFFER_HELPER_INISIZE);
	XTEST(buffer_extend(buffer, RINOO_BUFFER_HELPER_INISIZE + 42) == 0);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, RINOO_BUFFER_HELPER_INISIZE + 42));
	XTEST(buffer_extend(buffer, RINOO_BUFFER_HELPER_INISIZE * 2 + 42) == 0);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, RINOO_BUFFER_HELPER_INISIZE * 2 + 42));
	buffer_destroy(buffer);
	XPASS();
}
