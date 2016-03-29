#include "redis_status.h"

/* 名  称: save_to_redis
 * 功  能: 把用户在线状态存入redis
 * 参  数: p 指向用户id地址
 * 返回值: 0 表示成功，-1 表示失败
 */
int set_user_online(char *id, char *redis_address, int redis_port)
{
	int             User_online = 1;
	redisContext    *c = NULL;
	redisReply      *reply = NULL;
	char            *hostname = redis_address;
	int             port = redis_port;

	LOG(LOG_TO_REDIS, D, "set_user_online :%s\n", id);
	c = redisConnect(hostname, port);

	if ((c == NULL) || c->err) {
		x_printf(D, "connect error..... \n");
		LOG(LOG_TO_REDIS, D, "connect online error.....\n");
		redisFree(c);
		return -1;
	}

	reply = redisCommand(c, "sadd %s %s", "Accountids", id);

	if (reply != NULL) {
		if (reply->integer != 1) {
			x_printf(D, "integer error \n");
			LOG(LOG_TO_REDIS, D, "sadd  Accountids %s failed\n", id);
			freeReplyObject(reply);
			redisFree(c);
			return -1;
		}
	} else {
		x_printf(D, "sadd  reply  NULL\n");
		LOG(LOG_TO_REDIS, D, "sadd  reply  NULL:%s\n", id);
		freeReplyObject(reply);
		redisFree(c);
		return -1;
	}

	freeReplyObject(reply);

	reply = redisCommand(c, "hmset %s %s %d", id, "status", User_online);

	if (reply != NULL) {
		if (strcasecmp(reply->str, "OK") != 0) {
			x_printf(D, "str error \n");
			LOG(LOG_TO_REDIS, D, "hmset %s status %d\n", id, User_online);
			freeReplyObject(reply);
			redisFree(c);
			return -1;
		}
	} else {
		x_printf(D, "hmset reply NULL\n");
		LOG(LOG_TO_REDIS, D, "hmset reply NULL:%s\n", id);
		freeReplyObject(reply);
		redisFree(c);
		return -1;
	}

	freeReplyObject(reply);

	redisFree(c);

	LOG(LOG_TO_REDIS, D, "save to success:%s\n", id);

	return 0;
}

/* 名  称: change_to_redis
 * 功  能: 修改用户状态离线
 * 参  数: p 指向用户id地址
 * 返回值: 0 表示成功，-1 表示失败
 */
int set_user_offline(char *id, char *redis_address, int redis_port)
{
	int             offline = 0;
	redisContext    *c = NULL;
	redisReply      *reply = NULL;
	char            *hostname = redis_address;
	int             port = redis_port;

	LOG(LOG_TO_REDIS, D, "set_user_offline :%s\n", id);
	c = redisConnect(hostname, port);

	if ((c == NULL) || c->err) {
		x_printf(D, "connect error.....\n");
		LOG(LOG_TO_REDIS, D, "connect offline  error .....\n");
		redisFree(c);
		return -1;
	}

	reply = redisCommand(c, "srem %s %s", "Accountids", id);

	if (reply != NULL) {
		if (reply->integer != 1) {
			x_printf(D, "integer error \n");
			LOG(LOG_TO_REDIS, D, "srem Accountids %s failed\n", id);
			freeReplyObject(reply);
			redisFree(c);
			return -1;
		}
	} else {
		x_printf(D, "srem  reply  NULL\n");
		LOG(LOG_TO_REDIS, D, "srem  reply  NULL:%s\n", id);
		freeReplyObject(reply);
		redisFree(c);
		return -1;
	}

	freeReplyObject(reply);

	reply = redisCommand(c, "hmset %s %s %d", id, "status", offline);

	if (reply != NULL) {
		if (strcasecmp(reply->str, "OK") != 0) {
			x_printf(D, "hmset offline str error");
			LOG(LOG_TO_REDIS, D, "hmset %s status %d failed\n", id, offline);
			freeReplyObject(reply);
			redisFree(c);
			return -1;
		}
	} else {
		x_printf(D, "hmset offline reply  NULL");
		LOG(LOG_TO_REDIS, D, "hmset offline reply  NULL:%s\n", id);
		freeReplyObject(reply);
		redisFree(c);
		return -1;
	}

	freeReplyObject(reply);

	redisFree(c);
	LOG(LOG_TO_REDIS, D, "change to success:%s\n", id);
	return 0;
}

