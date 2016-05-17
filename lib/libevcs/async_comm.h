#pragma once

#include "http_api/http_status.h"
#include "redis_api/redis_status.h"

/** async protocol callback prototype */
typedef int (PROTO_CALL_BACK)(void *parse, ...);

enum proto_stat
{
	PROTO_STAT_FAIL = -1,
	PROTO_STAT_OMIT = 0,
	PROTO_STAT_DONE = 1
};

int http_proto_init(struct http_parse_info *info, char *const *data, unsigned const *size);

int http_proto_resp(struct http_parse_info *info);

int http_proto_reqt(struct http_parse_info *info);

int redis_proto_init(struct redis_parse_info *info, char *const *data, unsigned const *size);

int redis_proto_resp(struct redis_parse_info *info);

int redis_proto_reqt(struct redis_parse_info *info);
