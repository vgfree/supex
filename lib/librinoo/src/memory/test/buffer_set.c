/**
 * @file   buffer_add.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Sat Feb 8 18:27:58 2014
 *
 * @brief  buffer_set unit test
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
	t_buffer buffer;
	char memory[25];

	buffer_set(&buffer, memory, sizeof(memory));
	XTEST(buffer.ptr == memory);
	XTEST(buffer.class != NULL);
	XTEST(buffer.size == 0);
	XTEST(buffer.msize == sizeof(memory));
	XTEST(buffer_add(&buffer, "blablabla", 9) == 9);
	XTEST(buffer.ptr != NULL);
	XTEST(memcmp(memory, "blablabla", 9) == 0);
	XTEST(buffer.size == 9);
	XTEST(buffer.msize == sizeof(memory));
	XTEST(buffer_add(&buffer, "blablabla", 9) == 9);
	XTEST(buffer.ptr != NULL);
	XTEST(memcmp(memory, "blablablablablabla", 18) == 0);
	XTEST(buffer.size == 18);
	XTEST(buffer.msize == sizeof(memory));
	XTEST(buffer_add(&buffer, "blablabla", 9) == -1);
	XTEST(buffer.ptr != NULL);
	XTEST(buffer.size == 18);
	XTEST(buffer.msize == sizeof(memory));
	XPASS();
}
