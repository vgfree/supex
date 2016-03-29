#include "binheap.h"
#include <stdio.h>

int
main()
{
	PriorityQueue   H = Initialize(50);
	ElementType     ar[] = { { 0, 0, 32, 0, NULL }, { 0, 0, 21, 0, NULL }, { 0, 0, 16, 0, NULL }, { 0, 0, 24, 0, NULL },
				 { 0, 0, 31, 0, NULL }, { 0, 0, 19, 0, NULL }, { 0, 0, 68, 0, NULL },
				 { 0, 0, 65, 0, NULL }, { 0, 0, 26, 0, NULL }, { 0, 0, 13, 0, NULL } };
	int             i;

	for (i = 0; i < 10; i++) {
		Insert(ar[i], H);
	}

	for (i = 0; i < 10; i++) {
		printf("%d\n", DeleteMin(H).priority);
	}

	Destroy(H);
	return 0;
}

