/**
 * @file   vector_add.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Wed Apr 30 18:49:23 2014
 *
 * @brief  vector_add unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#if __WORDSIZE == 64
# define INT_TO_PTR(p) ((void *)(uint64_t)(p))
#else
# define PTR_TO_INT(p) ((int)(p))
#endif


/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	int i;
	size_t prev = 4;
	t_vector vector = { 0 };

	for (i = 0; i < 1000; i++) {
		XTEST(vector_add(&vector, INT_TO_PTR(i)) == 0);
		XTEST(vector.size == (size_t) i + 1);
		XTEST(vector.msize == prev * 2);
		if ((size_t) (i + 1) >= vector.msize) {
			prev = vector.msize;
		}
	}
	for (i = 0; i < 1000; i++) {
		XTEST(vector_get(&vector, (uint32_t) i) == INT_TO_PTR(i));
	}
	XTEST(vector_get(&vector, 1000) == NULL);
	vector_destroy(&vector);
	XPASS();
}
