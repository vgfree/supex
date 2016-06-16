#pragma once

#include "utils.h"

#define MAX_TEST_IMEI   32
#define MAX_LINK_INDEX  32

typedef struct rr_link
{
	short   port;
	char    host[64];
} rr_link;

struct rr_cfg_file
{
	struct rr_link  pmr_server;
	struct rr_link  trafficapi_server;
	struct rr_link  forward_server;
	struct rr_link  road_traffic_server;
	struct rr_link  city_traffic_server;
	struct rr_link  county_traffic_server;

	char            *model_cfg_name;
	// unsigned short     model_type;
	char            *appKey;
	char            *secret;
	int             save_time;
	int             kv_cache_count;
	int             redis_conn;
	long            imei_buff[MAX_TEST_IMEI];
	int             imei_count;
};

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name);

