/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    redis_interface.h
 * @detail  response results to pro from redis server.
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#ifndef __REDIS_INTERFACE_
#define __REDIS_INTERFACE_

#include "hiredis.h"
#include "redis_kv.h"
#include "libkv.h"

int connect_toredis(const char *server, const int port, redisContext **con_status);

void free_connect(redisContext *con_status);

int ping_redis(redisContext *con_status);

int loadhkeys_tofile(redisContext *con_status, const char *filename);
#endif

