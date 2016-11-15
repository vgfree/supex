/**
 * @file   buffer_vprint.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Jan 21 18:27:58 2010
 *
 * @brief  buffer_vprint unit test
 *
 *
 */

#include "rinoo/rinoo.h"

static t_buffer_class test_class = {
	.inisize = 10,
	.maxsize = RINOO_BUFFER_HELPER_MAXSIZE,
	.init = NULL,
	.growthsize = buffer_helper_growthsize,
	.malloc = buffer_helper_malloc,
	.realloc = buffer_helper_realloc,
	.free = buffer_helper_free,
};

int test_vprint(t_buffer *buffer, const char *format, ...)
{
	int res;
	va_list ap;

	va_start(ap, format);
	res = buffer_vprint(buffer, format, ap);
	va_end(ap);
	return res;
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_buffer *buffer;

	buffer = buffer_create(&test_class);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == 10);
	XTEST(test_vprint(buffer, "42 %s", "42") == 5);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "42 42", 5) == 0);
	XTEST(buffer->size == 5);
	XTEST(buffer->msize == 10);
	XTEST(test_vprint(buffer, " 42 %s %d", "42", 42) == 9);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "42 42 42 42 42", 14) == 0);
	XTEST(buffer->size == 14);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 14));
	buffer_destroy(buffer);
	test_class.inisize = 4;
	buffer = buffer_create(&test_class);
	XTEST(buffer != NULL);
	XTEST(buffer->ptr != NULL);
	XTEST(buffer->size == 0);
	XTEST(buffer->msize == 4);
	XTEST(test_vprint(buffer, "4242") == 4);
	XTEST(buffer->ptr != NULL);
	XTEST(memcmp(buffer->ptr, "4242", 4) == 0);
	XTEST(buffer->size == 4);
	XTEST(buffer->msize == buffer->class->growthsize(buffer, 4));
	buffer_destroy(buffer);
	XPASS();
}
