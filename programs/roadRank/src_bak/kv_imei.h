#ifndef __KV_IMEI_H_
#define __KV_IMEI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "libkv.h"
#include "decode_gps.h"

#define ERR_IMEI        -1
#define SUC_IMEI        0
#define NIL_IMEI        1

typedef struct kv_imei
{
	int     count;
	int     max_speed_num;
	int     max_speed;
	int     direction;
	long    str_time;
	long    end_time;
	long    roadID;
	long    IMEI;
	long    citycode;
	long    countycode;
} KV_IMEI;

int get_IMEI_from_kv(long IMEI, KV_IMEI *kv_IMEI);

int set_IMEI_to_kv(KV_IMEI *kv_IMEI);
#endif	/* ifndef __KV_IMEI_H_ */

