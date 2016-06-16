#ifndef __KV_ROAD_H_
#define __KV_ROAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rr_def.h"
#include "utils.h"
#include "libkv.h"

typedef struct road_section_t
{
	unsigned short  begin;
	unsigned short  end;
	unsigned short  avg_speed;
	long            endtime;
	double          longitude;		/*gps最新点经度*/
	double          latitude;		/*gps最新点纬度*/
} SITE;

typedef struct seckv_road
{
	char            shift;
	SITE            road_sec[ROADSECNUM];	// FIXME maybe not enough
	int             sec_num;
	int             max_speed;
	int             avg_speed;
	int             citycode;
	int             countycode;
	long long       IMEI;
	long            end_time;
	long            used_time;
	long            old_roadID;
} SECKV_ROAD;

int set_roadID_to_kv(SECKV_ROAD *kv_roadID);

int get_roadINFO_from_kv(long roadid, SECKV_ROAD *kv_road);

int set_secroad_to_kv(SECKV_ROAD *kv_road);

void parse_road_section(char *section, SECKV_ROAD *road);

void parse_road_section_s(char *section, SECKV_ROAD *road);

void show_road_section(char *ret, SECKV_ROAD *road);

void assembly_road_section(char *ret, SECKV_ROAD *road);

void single_road_section(char *ret, SECKV_ROAD *road);

// int set_redis_roadID(KV_ROADID *kv_roadID);
#endif	/* ifndef __KV_ROAD_H_ */

