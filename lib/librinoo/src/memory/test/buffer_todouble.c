/**
 * @file   buffer_todouble.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Apr  7 18:20:26 2011
 *
 * @brief  buffer_todouble unit test
 *
 *
 */

#include "rinoo/rinoo.h"

void check_buf(t_buffer *buffer, double expected_result)
{
	size_t len;
	double result;

	result = buffer_todouble(buffer, NULL);
	XTEST(result == expected_result);
	len = 0;
	result = buffer_todouble(buffer, &len);
	XTEST(result == expected_result);
	XTEST(len == buffer_size(buffer));
	result = buffer_todouble(buffer, NULL);
	XTEST(result == expected_result);
	len = 0;
	result = buffer_todouble(buffer, &len);
	XTEST(result == expected_result);
	XTEST(len == buffer_size(buffer));
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
	t_buffer buffer2;

	buffer = buffer_create(NULL);
	XTEST(buffer != NULL);
	XTEST(buffer_add(buffer, "12345.6789", 10) == 10);
	check_buf(buffer, 12345.6789);
	buffer_erase(buffer, 0);
	XTEST(buffer_add(buffer, "1", 1) == 1);
	check_buf(buffer, 1);
	buffer_erase(buffer, 0);
	XTEST(buffer_add(buffer, "-12345.678", 10) == 10);
	check_buf(buffer, -12345.678);
	buffer_erase(buffer, 0);
	buffer_destroy(buffer);
	strtobuffer(&buffer2, "98765.4321");
	check_buf(&buffer2, 98765.4321);
	strtobuffer(&buffer2, "0");
	check_buf(&buffer2, 0);
	strtobuffer(&buffer2, "-9876543.21");
	check_buf(&buffer2, -9876543.21);
	XPASS();
}
