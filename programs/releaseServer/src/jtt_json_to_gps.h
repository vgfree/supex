#pragma once
#include "jtt_body.h"
#include "cJSON.h"

/*
 * bgps:为实时数据
 * bextra:为补传数据，空间由内部malloc，要求外部free
 * 说明：
 *	*bextra == NULL:表明没有补传数据
 * */
int json_to_gps_data(cJSON *str, BODY_GPS *bgps, int *n_gps, BODY_GPS **bextra, int *n_extra);

