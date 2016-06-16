#pragma once

#define IMEI_LEN                15
#define MERGED_LIMIT            5
#define EARTH_RADIUS            6378.137

#define ASYNC_LIBEV_THRESHOLD   6
#define ASYNC_LUAKV_THRESHOLD   1
#define BUFF_USE_LEN            10240
#define ROADSECNUM              100

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

