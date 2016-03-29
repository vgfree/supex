#ifndef __MATCH_ROAD_H_
#define __MATCH_ROAD_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <match.h>
#include <ev.h>

#include "utils.h"
#include "pool_api.h"
#include "async_api.h"
#include "decode_gps.h"

typedef struct road_info
{
    short     segmentID;
    short     rt;
    int    citycode;
    int    countycode;
    long    road_rootID;
    long    new_roadID;
} ROAD_INFO;

int match_road(struct ev_loop *loop, CAL_INFO *cal_info);

int get_roadId(char *http_data, ROAD_INFO **road_info,
	int direction);
#endif	/* ifndef __MATCH_ROAD_H_ */

