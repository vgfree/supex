#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct comm_sds
{
	char    *str;
	size_t  len;
	size_t  cap;
	bool    freeable;
} comm_sds_t;

struct comm_sds *commsds_make(struct comm_sds *sds, size_t cap);

void commsds_free(struct comm_sds *sds);

void commsds_clean(struct comm_sds *sds);

void commsds_expand(struct comm_sds *sds, size_t len);

void commsds_push_head(struct comm_sds *sds, const char *str, int len);

void commsds_pull_head(struct comm_sds *sds, const char *str, int len);

void commsds_push_tail(struct comm_sds *sds, const char *str, int len);

void commsds_pull_tail(struct comm_sds *sds, const char *str, int len);

void commsds_copy(struct comm_sds *dst, struct comm_sds *src);

struct comm_sds *commsds_create(const char *str, size_t len);

