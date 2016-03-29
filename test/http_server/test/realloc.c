#include <stdlib.h>

int main()
{
	char *buf = calloc(100, 1);

	free(buf);
	buf = NULL;
	buf = realloc(buf, 100);
	return 0;
}

