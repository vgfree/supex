/**
 * @file   buffer_cmp.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Jan 21 18:27:58 2010
 *
 * @brief  buffer_cmp unit test
 *
 *
 */

#include "rinoo/rinoo.h"

char *strings[] = {
	"aabb",
	"aac",
	"ba",
	"baa",
	"bb",
	"c",
	"cccccccccccccccccc"
};

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	uint32_t i;
	t_buffer *buffer1;
	t_buffer *buffer2;

	buffer1 = buffer_create(NULL);
	XTEST(buffer1 != NULL);
	buffer2 = buffer_create(NULL);
	XTEST(buffer2 != NULL);
	for (i = 0; i < ARRAY_SIZE(strings) - 1; i++) {
		buffer_add(buffer1, strings[i], strlen(strings[i]));
		buffer_add(buffer2, strings[i + 1], strlen(strings[i + 1]));
		XTEST(buffer_cmp(buffer1, buffer2) < 0);
		buffer_erase(buffer1, buffer_size(buffer1));
		buffer_erase(buffer2, buffer_size(buffer2));
	}
	buffer_destroy(buffer1);
	buffer_destroy(buffer2);
	XPASS();
}
