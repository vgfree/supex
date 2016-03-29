/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    redis_interface.c
 * @detail  response results to pro from redis server.
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "redis_interface.h"

const char      *PROVINCEN = "provinceName";
const char      *PROVINCEC = "provinceCode";
const char      *CITYN = "cityName";
const char      *CITYC = "cityCode";
const char      *COUNTYN = "countyName";
const char      *COUNTYC = "countyCode";

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

int loadhkeys_tofile(redisContext *con_status, const char *filename)
{
	if (!con_status) {
		printf("Parameters can not be empty");
		return -1;
	}

	FILE *fp;
	fp = fopen(filename, "w+");

	if (!fp) {
		printf("open error. !\n");
		return -1;
	}

	redisReply      *reply;
	redisReply      *current;
	reply = redisCommand(con_status, "KEYS *");

	int i;

	while (reply->element[i]) {
		char buf[27] = { '\0' };
		// printf("keys:%s\n", (*reply->element[i]).str);
		sprintf(buf, "HGET %s %s", (*reply->element[i]).str, CITYC);
		current = redisCommand(con_status, buf);

		if (current) {
			if (strlen(reply->element[i]->str) == 9) {
				fprintf(fp, "%s%c%s", reply->element[i]->str, '\0', current->str);
				// fseek(fp, 16, SEEK_CUR);
			} else if (strlen(reply->element[i]->str) == 10) {
				fprintf(fp, "%s%s", reply->element[i]->str, current->str);
				// fseek(fp, 16, SEEK_CUR);
			}
		}

		i++;
		freeReplyObject(current);
	}

	fclose(fp);
	freeReplyObject(reply);
	return 0;
}

