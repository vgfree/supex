#ifndef __KV_IMEI_H_
#define __KV_IMEI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rr_def.h"
#include "utils.h"
#include "libkv.h"

typedef struct kv_imei
{
	unsigned short  count;
	unsigned short  rt;
	int             max_speed_num;
	int             max_speed;
	int             direction;
	long            str_time;
	long            end_time;
	long            roadID;
	char            IMEI[IMEI_LEN];
	long            citycode;
	long            countycode;

	double          start_lon;
	double          start_lat;
	double          end_lon;
	double          end_lat;
} KV_IMEI;

int get_IMEI_from_kv(char *IMEI, KV_IMEI *kv_IMEI);

int set_IMEI_to_kv(KV_IMEI *kv_IMEI);

int delete_IMEI_from_kv(KV_IMEI *kv_IMEI);

// int set_IMEI_expire(KV_IMEI *kv_IMEI, int time);
#endif	/* ifndef __KV_IMEI_H_ */

