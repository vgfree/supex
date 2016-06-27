#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

#define XMQ_U64_MAX     "18446744073709551615"
#define XMQ_U64_IDX     "00000000073709551615"

#define XMQ_U64_MAX_NB  (18446744073709551615UL)
#define XMQ_U64_IDX_NB  (73709551615UL)

/*
 * XMQ_KEY_LEN 21  "18446744073709551615"  __u64toa()/__atou64() to change.
 * __atou64, string type with "00000000000002378923" convert to uint64_t,
 * len = strlen(src) or the length you want to convert, from src.
 */
static uint64_t __atou64(const char *str, size_t len);

/* __u64toa, uint64_t type change to string "00000000000002378923" */
static char *__u64toa(uint64_t u64, char *des, int len);

uint64_t __atou64(const char *str, size_t len)
{
	// return_if_false((str && str[0] != '\0' && len > 0), (uint64_t)-1);
	if (str && (str[0] != '\0') && (len > 0)) {} else {
		return (uint64_t)-1;
	}

	uint64_t        t = 1, des = 0;
	const char      *p = str + len - 1;

	for (; p >= str; p--) {
		des += t * (*p - 48);
		// printf("'%c' [%d] [%llu] des:%llu\n", *p, *p - 48, t*(*p-48), des);
		t *= 10;
	}

	return des;
}

char *__u64toa(uint64_t u64, char *des, int len)
{
	// return_if_false((des && len > 0), (char *)-1);
	if (des && (len > 0)) {} else {
		return (char *)-1;
	}

	int i;

	for (i = len - 2; i >= 0; i--) {
		des[i] = 48 + (u64 % 10);
		u64 /= 10;
	}

	des[len - 1] = '\0';

	return des;
}

void main(void)
{
	unsigned long long int  i = strtoull(XMQ_U64_MAX, NULL, 10);
	uint64_t                j = __atou64(XMQ_U64_MAX, strlen(XMQ_U64_MAX));

	printf("%llu\n", i);
	printf("%llu\n", j);
	printf("%llu\n", ULLONG_MAX);
	unsigned long long int  x = strtoull(XMQ_U64_IDX, NULL, 0);
	unsigned long long int  y = strtoull(XMQ_U64_IDX, NULL, 10);
	printf("%llu\n", x);
	printf("%llu\n", y);

	char k1[21];
	__u64toa(ULLONG_MAX, k1, sizeof(k1));
	printf("%s\n", k1);
	char k2[21];
	__u64toa(XMQ_U64_IDX_NB, k2, sizeof(k2));
	printf("%s\n", k2);

	char k3[21];
	snprintf(k3, sizeof(k3), "%020llu", ULLONG_MAX);
	printf("%s\n", k3);
	char k4[21] = { 'a', 'b' };
	printf("%s\n", k4);
	snprintf(k4, sizeof(k4), "%020llu", XMQ_U64_IDX_NB);
	printf("%s\n", k4);
}

