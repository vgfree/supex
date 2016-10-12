#pragma once

#define IMEI_LEN                36
#define MERGED_LIMIT            5
#define EARTH_RADIUS            6378.137

#define ASYNC_LIBEV_THRESHOLD   10
#define ASYNC_LUAKV_THRESHOLD   1
#define BUFF_USE_LEN            6200
#define BUFF_SEC_LEN            5660
#define ROADSECNUM              100
#define REDIS_ERR               -1
#define REDIS_OK                0
#define MAX_TEST_IMEI           32
#define MAX_LINK_INDEX          32

enum _error_imei
{
	ERR_IMEI = -1,
	SUC_IMEI = 0,
	NIL_IMEI = 1
};

enum _error_road_kv
{
	ERR_ROAD = -1,
	SUC_ROAD = 0,
	NIL_ROAD = 1
};

typedef enum _modeltype
{
	SINGLE = 0,
	SUBSEV
} MODEL_TYPE;

enum _road_rank
{
	HIGHWAY = 0,
	EXPRESSWAY = 10,
	LOWWAY
};

