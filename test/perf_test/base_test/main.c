#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void longa()
{
	int     i = 0;
	int     j = 0;

	for (i = 0; i < 1000000; i++) {
		j = i;	// am I silly or crazy? I feel boring and desperate.
	}
}

void foo2()
{
	int i;

	for (i = 0; i < 10; i++) {
		longa();
	}
}

void foo1()
{
	int i;

	for (i = 0; i < 100; i++) {
		longa();
	}
}

int main(void)
{
	foo1();
	foo2();
	return EXIT_SUCCESS;
}

