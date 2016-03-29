#ifndef __KV_ROAD_H_
#define __KV_ROAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "libkv.h"
#include "decode_gps.h"

typedef struct kv_roadID
{
    char    shift;
    int     max_speed;
    int     avg_speed;
    int    citycode;
    int    countycode;
    long long    IMEI;
	long    end_time;
	long    used_time;
    long    old_roadID;
} KV_ROADID;

int set_roadID_to_kv(KV_ROADID *kv_roadID);

// int set_redis_roadID(KV_ROADID *kv_roadID);
#endif	/* ifndef __KV_ROAD_H_ */

