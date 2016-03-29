/**
 * Author       : chenzutao
 * Date         : 2016-01-27
 * Function     : redis operate on topology by C language
 **/

#include "redis_operation.h"
#include <graphlab/logger/logger_includes.hpp>

redisContext *redis_connect(const char *host, short port)
{
	struct timeval  _tv = { 3, 0 };
	redisContext    *conn = redisConnectWithTimeout(host, port, _tv);

	if ((NULL != conn) && conn->err) {
		logger(LOG_ERROR, "[** FAILED **] Failed to connect to redis server(%s:%d) !\n", host, port);
		redisFree(conn);
		return NULL;
	}

	return conn;
}

int redis_set(redisContext *conn, const char *key, const char *value)
{
	if ((key == NULL) || (value == NULL)) {
		logger(LOG_ERROR, "set redis key or value error !\n");
		return -1;
	}

	if (conn == NULL) {
		logger(LOG_ERROR, "redis conn error !\n");
		return -1;
	}

	char *cmd_str = NULL;

	cmd_str = (char *)malloc(strlen(key) + strlen(value) + 10);

	if (cmd_str == NULL) {
		logger(LOG_ERROR, "malloc cmd_str error !\n");
		return -1;
	}

	sprintf(cmd_str, "SET %s %s", key, value);

	redisReply *reply = (redisReply *)redisCommand(conn, cmd_str);
	logger(LOG_INFO, "execute command [%s] !\n", cmd_str);

	if (!((NULL != reply) && (reply->type == REDIS_REPLY_STATUS) && (strcasecmp(reply->str, "OK") == 0))) {
		logger(LOG_ERROR, "[** FAILED **] Failed to execute command [%s] !\n", cmd_str);
		free(cmd_str), cmd_str = NULL;
		freeReplyObject(reply);
		return -1;
	}

	free(cmd_str), cmd_str = NULL;
	freeReplyObject(reply);
	return 0;
}

char *redis_get(redisContext *conn, const char *key)
{
	if ((key == NULL) || (conn == NULL)) {
		logger(LOG_ERROR, "redis connection or key error !\n");
		return NULL;
	}

	char *cmd_str = NULL;
	cmd_str = (char *)malloc(strlen(key) + 10);

	if (cmd_str == NULL) {
		logger(LOG_ERROR, "malloc cmd_str error !\n");
		return NULL;
	}

	sprintf(cmd_str, "GET %s", key);
	redisReply *reply = (redisReply *)redisCommand(conn, cmd_str);
	logger(LOG_INFO, "execute command [%s] !\n", cmd_str);

	if ((NULL != reply) && (reply->type == REDIS_REPLY_STRING) && (reply->str != NULL)) {
		int     len = strlen(reply->str);
		char    *value = (char *)calloc(len + 1, sizeof(char));

		if (value == NULL) {
			logger(LOG_ERROR, "malloc get redis value error!\n");
			free(cmd_str), cmd_str = NULL;
			freeReplyObject(reply);
			return NULL;
		}

		strncpy(value, reply->str, len);
		free(cmd_str), cmd_str = NULL;
		freeReplyObject(reply);
		return value;
	} else {
		logger(LOG_ERROR, "[** FAILED **] Failed to execute command [GET %s] !\n", key);
		free(cmd_str), cmd_str = NULL;
		freeReplyObject(reply);
		return NULL;
	}
}

int redis_ping(redisContext *conn)
{
	if (NULL == conn) {
		logger(LOG_ERROR, "redis ping connection error !\n");
		return 0;
	}

	redisReply *reply = (redisReply *)redisCommand(conn, "PING");

	if (reply && (strcasecmp(reply->str, "PONG") == 0)) {
		freeReplyObject(reply);
		return 1;
	}

	freeReplyObject(reply);
	return 0;
}

long get_from_nodeID_by_roadID(redisContext *conn, long roadID)
{
	if (NULL == conn) {
		logger(LOG_ERROR, "get from nodeID redis connection error !\n");
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(conn, "HMGET FNBR %ld", roadID);

	if (NULL == reply) {
		logger(LOG_ERROR, "get from nodeID reply is NULL !\n");
		return -1;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		return atol(reply->element[0]->str);
	}

	return -1;
}

long get_end_nodeID_by_roadID(redisContext *conn, long roadID)
{
	if (NULL == conn) {
		logger(LOG_ERROR, "get end nodeID redis connection error !\n");
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(conn, "HMGET ENBR %ld", roadID);

	if (NULL == reply) {
		logger(LOG_ERROR, "get end nodeID reply is NULL");
		return -1;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		return atol(reply->element[0]->str);
	}

	return -1;
}

redisReply *get_import_roadID(redisContext *conn, long nodeID)
{
	if (NULL == conn) {
		logger(LOG_ERROR, "get import roadID redis connection error !\n");
		return NULL;
	}

	redisReply *reply = (redisReply *)redisCommand(conn, "HMGET IROF %ld", nodeID);

	if (NULL == reply) {
		logger(LOG_ERROR, "get import roadID reply is NULL !\n");
		return NULL;
	}

	return reply;
}

redisReply *get_export_roadID(redisContext *conn, long nodeID)
{
	if (NULL == conn) {
		logger(LOG_ERROR, "get export roadID redis connection error !\n");
		return NULL;
	}

	// redisReply *reply = (redisReply *)redisCommand(conn, "HMGET ERBR %ld", nodeID);
	redisReply *reply = (redisReply *)redisCommand(conn, "HMGET EROF %ld", nodeID);

	if (NULL == reply) {
		logger(LOG_ERROR, "get export roadID reply is NULL !\n");
		return NULL;
	}

	return reply;
}

/*
 *   REDIS_REPLY_STRING 1
 *   REDIS_REPLY_ARRAY 2
 *   REDIS_REPLY_INTEGER 3
 *   REDIS_REPLY_NIL 4
 *   REDIS_REPLY_STATUS 5
 *   REDIS_REPLY_ERROR 6
 */
/**/
#if 0
int main(int argc, char **argv)
{
	redisContext *conn = NULL;

	// conn = redis_connect("192.168.1.196", 5401);
	conn = redis_connect("192.168.1.10", 4060);
	long            roadID = 76120209;
	redisReply      *reply = get_export_roadID(conn, roadID);

	if (reply == NULL) {
		logger(LOG_ERROR, "reply is NULL !\n");
		return -1;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		int i = 0;

		for (i = 0; i < reply->elements; ++i) {
			printf("%d --> %s\n", i, reply->element[i]->str);
		}
	}

	freeReplyObject(reply);

	/*
	 *   //redis_set(conn, "AAA", "aAaAaA");
	 *   char *value = NULL;
	 *   value = redis_get(conn, "AAA");
	 *   printf("%s\n", value);
	 *   //puts(value);
	 *   free(value);
	 */
	redisFree(conn);
	return 0;
}
#endif	// if 0
/**/

