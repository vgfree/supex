/**
 * @file   buffer_tostr.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Apr  7 18:20:26 2011
 *
 * @brief  buffer_tostr unit test
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
	char *result;

	buffer = buffer_create(NULL);
	XTEST(buffer != NULL);
	XTEST(buffer_add(buffer, "qwertyuiop", 10) == 10);
	result = buffer_tostr(buffer);
	XTEST(result != NULL);
	XTEST(strlen(result) == 10);
	XTEST(strcmp(result, "qwertyuiop") == 0);
	XTEST(buffer_size(buffer) == 11);
	buffer_destroy(buffer);
	XPASS();
}
