/**
 * @file   buffer_tolong.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Apr  7 18:20:26 2011
 *
 * @brief  buffer_tolong unit test
 *
 *
 */

#include "rinoo/rinoo.h"

void check_buf(t_buffer *buffer, long int expected_result)
{
	size_t len;
	long int result;

	result = buffer_tolong(buffer, NULL, 0);
	XTEST(result == expected_result);
	len = 0;
	result = buffer_tolong(buffer, &len, 0);
	XTEST(result == expected_result);
	XTEST(len == buffer_size(buffer));
	result = buffer_tolong(buffer, NULL, 10);
	XTEST(result == expected_result);
	len = 0;
	result = buffer_tolong(buffer, &len, 10);
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
	XTEST(buffer_add(buffer, "123456789", 9) == 9);
	check_buf(buffer, 123456789);
	buffer_erase(buffer, buffer_size(buffer));
	XTEST(buffer_add(buffer, "1", 1) == 1);
	check_buf(buffer, 1);
	buffer_erase(buffer, buffer_size(buffer));
	XTEST(buffer_add(buffer, "-12345678", 9) == 9);
	check_buf(buffer, -12345678);
	buffer_erase(buffer, buffer_size(buffer));
	buffer_destroy(buffer);
	strtobuffer(&buffer2, "987654321");
	check_buf(&buffer2, 987654321);
	strtobuffer(&buffer2, "0");
	check_buf(&buffer2, 0);
	strtobuffer(&buffer2, "-987654321");
	check_buf(&buffer2, -987654321);
	XPASS();
}
