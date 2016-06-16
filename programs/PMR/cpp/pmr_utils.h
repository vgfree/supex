#pragma once

#include "cache/cache.h"
#include "major/smart_api.h"
#include <stdio.h>
#include <stdint.h>

int get_number_len(uint64_t len);

int put_string_out(struct cache *p_cache, char *str);

int put_number_out(struct cache *p_cache, uint64_t nb);

int put_double_out(struct cache *p_cache, double d);

void send_error(struct cache *p_cache, char *p_error);

int check_parameter_locate(struct data_node *p_node, double *p_lon, double *p_lat, short *p_dir);

