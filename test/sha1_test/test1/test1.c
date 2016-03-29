#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define DATA_A "ksadkaskjdlkpqwokeqwlwqmwq.le,wqe.wqe;wqewwdewq,mqqqqqqqqqqqqqqqqqqqqqqqqqqokwemwemwm"

char *sha1_hash(const void *src, unsigned long len, char *dst)
{
	if (!dst) {
		dst = (char *)calloc(64, sizeof(char));
	}

	assert(dst);

	unsigned char md[21] = { 0 };
	SHA1(src, len, md);
	// printf("%s\n", md);

	int i = 0;

	for (i = 0; i < 20; i++) {
		snprintf(&dst[2 * i], 3, "%02X", (unsigned char)md[i]);
	}

	// printf("%s\n", dst);
	return dst;
}

void test(void)
{
	char temp[64] = { 0 };

	sha1_hash(DATA_A, strlen(DATA_A), temp);
}

void main(void)
{
	int i = 0;

	// for (i=0; i< 1000000; i++){
	test();
	// }
}

