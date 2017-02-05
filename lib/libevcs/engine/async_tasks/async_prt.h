#pragma once
#include "../proto_comm.h"

enum proto_type
{
	PROTO_TYPE_HTTP_RESP = 0,
	PROTO_TYPE_HTTP_REQT,
	PROTO_TYPE_REDIS_RESP,
	PROTO_TYPE_REDIS_REQT,
	PROTO_TYPE_SURE_RESP,
	PROTO_TYPE_SURE_REQT,
};

enum proto_attr
{
	PROTO_ATTR_SURE = 0,
	PROTO_ATTR_VARY
};

struct async_prt {
	union
	{
		struct http_parse_info  http_info;
		struct redis_parse_info redis_info;
		struct sure_parse_info sure_info;
	};
};
