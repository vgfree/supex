#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"
#include "mttpsvp_redis.h"
#include "sniff_evcoro_lua_api.h"

static redisContext *redis_ctx;

void mttpsvp_redis_init(const char *hostname, int port) {
  struct timeval timeout = {2, 0}; // 2 seconds
  redis_ctx = redisConnectWithTimeout(hostname, port, timeout);
  if (redis_ctx == NULL) {
		x_printf(E, "Redis connection error: not enough memory for redis context\n");
		exit(EXIT_FAILURE);
  }

  if (redis_ctx->err) {
		x_printf(E, "Redis connection error: %s\n", redis_ctx->errstr);
    redisFree(redis_ctx);
		exit(EXIT_FAILURE);
  }
}

void mttpsvp_redis_destory() {
  if (redis_ctx) {
    redisFree(redis_ctx);
  }
}

int mttpsvp_redis_check_gpstoken(const char *mirrtalk_id, const char *gpstoken, size_t gpstoken_len) {
  redisReply *reply = redisCommand(redis_ctx, "GET gpsToken:%s", mirrtalk_id);

  if (reply == NULL) {
		x_printf(E, "mttpsvp_redis_check_gpstoken redisCommand error: %s\n", redis_ctx->errstr);
    return -1;
  }

  int ret = 0;
  if (reply->len != gpstoken_len || strncmp(reply->str, gpstoken, gpstoken_len) != 0) {
    ret = -1;
  }

  freeReplyObject(reply);
  return ret;
}

void mttpsvp_redis_del_gpstoken(const char *mirrtalk_id) {
  redisReply *reply = redisCommand(redis_ctx, "DEL gpsToken:%s", mirrtalk_id);
  
  if (reply == NULL) {
		x_printf(E, "mttpsvp_redis_del_gpstoken redisCommand error: %s\n", redis_ctx->errstr);
  }

  freeReplyObject(reply);
}
