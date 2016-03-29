/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    laodredis.c
 * @detail  Convert redis to libkv.
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hiredis.h"
#include "redis_tomem.h"
#include "libkv.h"

const char      *HOST = "192.168.71.61";
const int       PORT = 5072;
const char      *PROVINCEN = "provinceName";
const char      *PROVINCEC = "provinceCode";
const char      *CITYN = "cityName";
const char      *CITYC = "cityCode";
const char      *COUNTYN = "countyName";
const char      *COUNTYC = "countyCode";

redisReply *hvalue(redisReply reply, char *buf, const char *cmd, redisContext *con_status, kv_handler_t *handler);

// connect to redis server ; success return 0
int connect_toredis(const char *server, const int port, redisContext **con_status)
{
	if (!server || (port < 0)) {
		printf("redis server or port error. !");
		return -1;
	}

	// timeout 1.5 sec
	struct timeval timeout = { 1, 500000 };

	// redisContext *con_status = NULL;
	*con_status = redisConnectWithTimeout(server, port, timeout);

	if (!(*con_status) || (*con_status)->err) {
		printf("Connection ot redis error. !\n");
		return -1;
	}

	return 0;
}

void free_connect(redisContext *con_status)
{
	redisFree(con_status);
}

// success return 0
int ping_redis(redisContext *con_status)
{
	if (!con_status) {
		printf("Parameters can not be empty");
		return -1;
	}

	int             ret = 0;
	redisReply      *reply;
	reply = redisCommand(con_status, "PING");

	if (strcmp(reply->str, "PONG")) {
		ret = 0;
	} else {
		ret = -1;
	}

	freeReplyObject(reply);
	return ret;
}

// convert redis server data to mem; success return 0
int convert_tomem(redisContext *con_status, const char *comd, kv_handler_t *handler)
{
	if (!con_status) {
		printf("Parameters can not be empty");
		return -1;
	}

	redisReply      *reply;
	redisReply      *current;
	reply = redisCommand(con_status, comd);

	int i;

	if (reply->type == REDIS_REPLY_ARRAY) {
		// while (i < 7) {
		while (reply->element[i]) {
			char *buf = (char *)malloc((*reply->element[i]).len + 19);

			if (!buf) {
				printf("malloc error. !\n");
				return -1;
			}

			printf("keys:%s\n", (*reply->element[i]).str);
			// sprintf(buf,"HGET %s %s",(*reply->element[i]).str, PROVINCEN);
			// current = redisCommand(con_status, buf);
			current = hvalue((*reply->element[i]), buf, PROVINCEN, con_status, handler);
			current = hvalue((*reply->element[i]), buf, PROVINCEC, con_status, handler);
			current = hvalue((*reply->element[i]), buf, CITYN, con_status, handler);
			current = hvalue((*reply->element[i]), buf, CITYC, con_status, handler);
			current = hvalue((*reply->element[i]), buf, COUNTYN, con_status, handler);
			current = hvalue((*reply->element[i]), buf, COUNTYC, con_status, handler);
			free(buf);
			freeReplyObject(current);
			i++;
		}
	} else {
		printf("expect type hash. !\n");
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

// put into mem; failed return NULL
redisReply *hvalue(redisReply reply, char *buf, const char *cmd, redisContext *con_status, kv_handler_t *handler)
{
	if (!buf || !cmd || !con_status) {
		printf("Parameters can not be empty");
		return NULL;
	}

	kv_answer_t     *ans;
	redisReply      *current;
	sprintf(buf, "HGET %s %s", reply.str, cmd);
	current = redisCommand(con_status, buf);

	char *hsetcmd = (char *)malloc(reply.len + 100);

	if (!hsetcmd) {
		printf("malloc error. !\n");
		return NULL;
	}

	int num = sprintf(hsetcmd, "hset %s %s %s", reply.str, cmd, current->str);
	libkv_cmd(handler, hsetcmd, num, &ans);
	// printf("vaules: %s\n",current->str);

	// num = sprintf(hsetcmd,"hget %s %s",reply.str, cmd);
	// libkv_cmd(handler,hsetcmd, num, &ans);
	// get_allhashvaule(ans);
	kvanswer_free(ans);
	free(hsetcmd);
	return current;
}

int main(int argc, char **argv)
{
	// unsigned int j;
	redisContext *con_status;

	// redisReply *reply;

	printf("\nused_memory before:%d\n", kv_get_used_memory());
	kv_handler_t *handler;
	handler = kv_create(NULL);

	connect_toredis(HOST, PORT, &con_status);
	convert_tomem(con_status, "KEYS *", handler);

	kvhandler_destroy(handler);
	free_connect(con_status);
	printf("\nused_memory before:%d\n", kv_get_used_memory());
	return 0;
}

