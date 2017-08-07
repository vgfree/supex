#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "comm_sds.h"

struct comm_sds *commsds_make(struct comm_sds *sds, size_t cap)
{
	if (sds) {
		sds->freeable = false;
	} else {
		sds = calloc(1, sizeof(struct comm_sds));
		sds->freeable = true;
	}

	sds->str = calloc(cap, sizeof(char));
	sds->cap = cap;
	sds->len = 0;

	return sds;
}

void commsds_free(struct comm_sds *sds)
{
	assert(sds);

	if (sds->str) {
		free(sds->str);
		sds->str = NULL;
		sds->len = 0;
		sds->cap = 0;
	}

	if (sds->freeable) {
		free(sds);
	}
}

void commsds_clean(struct comm_sds *sds)
{
	sds->len = 0;
}

void commsds_expand(struct comm_sds *sds, size_t len)
{
	size_t big = sds->len + len;

	if (big > sds->cap) {
		char *tmp = realloc(sds->str, big);

		if (tmp != NULL) {
			sds->str = tmp;
			sds->cap = big;
			memset(tmp + sds->len, 0, big - sds->len);
		}
	}
}

void commsds_push_core(struct comm_sds *sds, const char *str, int len, int off)
{
	assert((off >= 0) && (off <= sds->len));
	commsds_expand(sds, len);
	memmove(sds->str + off + len, sds->str + off, sds->len - off);

	if (str) {
		memcpy(sds->str + off, str, len);
	}

	sds->len += len;
}

void commsds_pull_core(struct comm_sds *sds, const char *str, int len, int off)
{
	assert((off >= 0) && (off <= sds->len));
	assert(len <= (sds->len - off));

	if (str) {
		memcpy((void *)str, (const void *)sds->str + off, len);
	}
	memmove(sds->str + off, sds->str + off + len, sds->len - off - len);
	sds->len -= len;
}

void commsds_push_head(struct comm_sds *sds, const char *str, int len)
{
	commsds_expand(sds, len);
	memmove(sds->str + len, sds->str, sds->len);

	if (str) {
		memcpy(sds->str, str, len);
	}

	sds->len += len;
}

void commsds_pull_head(struct comm_sds *sds, const char *str, int len)
{
	assert(len <= sds->len);

	if (str) {
		memcpy((void *)str, (const void *)sds->str, len);
	}
	memmove(sds->str, sds->str + len, sds->len - len);
	sds->len -= len;
}

void commsds_push_tail(struct comm_sds *sds, const char *str, int len)
{
	commsds_expand(sds, len);

	if (str) {
		memcpy(sds->str + sds->len, str, len);
	}

	sds->len += len;
}

void commsds_pull_tail(struct comm_sds *sds, const char *str, int len)
{
	assert(len <= sds->len);

	if (str) {
		memcpy((void *)str, (const void *)(sds->str + sds->len - len), len);
	}
	sds->len -= len;
}

void commsds_copy(struct comm_sds *dst, struct comm_sds *src)
{
	commsds_clean(dst);
	commsds_push_tail(dst, src->str, src->len);
}

struct comm_sds *commsds_create(const char *str, size_t len)
{
	struct comm_sds *sds = commsds_make(NULL, len);

	commsds_push_tail(sds, str, len);
	return sds;
}

