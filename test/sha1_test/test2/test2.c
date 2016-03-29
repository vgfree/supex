#include <stdio.h>
#include <string.h>
#include "sha1.h"

#define DATA_A "ksadkaskjdlkpqwokeqwlwqmwq.le,wqe.wqe;wqewwdewq,mqqqqqqqqqqqqqqqqqqqqqqqqqqokwemwemwm"

void main()
{
	char temp[1024] = { 0 };

	SHA1(temp, DATA_A, strlen(DATA_A));
	printf("%s\n", temp);

	int     i = 0;
	char    buf[1024] = { 0 };

	for (i = 0; i < 20; i++) {
		snprintf(&buf[2 * i], 3, "%02X", (unsigned char)temp[i]);
	}

	printf("%s\n", buf);
}

