/**
 * @file   vector_remove.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Wed Apr 30 18:49:48 2014
 *
 * @brief  vector_add unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#if __WORDSIZE == 64
# define INT_TO_PTR(p) ((void *)(uint64_t)(p))
#else
# define INT_TO_PTR(p) ((void *)(p))
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
	for (i = 0; i < 500; i++) {
		XTEST(vector_remove(&vector, (uint32_t) i) == 0);
	}
	XTEST(vector_size(&vector) == 500);
	for (i = 0; i < 500; i++) {
		XTEST(vector_get(&vector, (uint32_t) i) == INT_TO_PTR(i * 2 + 1));
	}
	vector_destroy(&vector);
	XPASS();
}
