#ifndef __DECODE_DATA__
#define __DECODE_DATA__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "utils.h"
#include "cJSON.h"

typedef struct _GPS_DATA_T
{
	char            *id;
	unsigned int    lat[5];
	unsigned int    lng[5];
	int             speed[5];
	int             angle[5];
	int             gpsTime[5];
} gps_data_t;

int decode_data(gps_data_t **value, const char *data);

int free_data(gps_data_t *value);
#endif	/* ifndef __DECODE_DATA__ */

