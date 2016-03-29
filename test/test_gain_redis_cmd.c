#include <stdio.h>
#include <string.h>

long change_number(char *string)
{
	int     i = 0;
	int     len = strlen(string);
	long    cmd = 0;

	for (i = 0; i < len; i++) {
		if (*(string + i) > 'Z') {
			*(string + i) -= 32;	/* 'A' - 'a' */
		}

		cmd = cmd * 26 + (*(string + i) - 'A');	/* A~Z is 26 numbers */
	}

	return cmd;
}

void main(void)
{
	char cmd[] = "publish";

	printf("%lld\n", change_number(cmd));
}

