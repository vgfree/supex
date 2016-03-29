#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <hiredis/hiredis.h>
#include "libkv.h"

void *set_data_to_kv(char *key, char *value, kv_handler_t *handler)
{
	int             i;
	char            strSet[100] = "hset ";
	kv_answer_t     *ans;

	strcat(strSet, key);
	strcat(strSet, " ");
	const char *cmd = strcat(strSet, value);
	printf("cmd:%s\n", cmd);

	ans = kv_ask(handler, cmd, strlen(cmd));

	kv_answer_value_t *return_value;
	return_value = kv_answer_first_value(ans);

	for (i = 0; i < return_value->ptrlen; i++) {
		printf("%c", ((char *)return_value->ptr)[i]);
	}

	printf("\n");

	kv_answer_release(ans);

	return NULL;
}

/*
 *        功能：程序重起从reidis中读取初始化数据到libkv中
 *        参数：无
 *        返回值：无
 *        日期：2015-11-11 吉中
 */
void init_data_to_kv()
{
	int             i;
	int             j;
	redisContext    *c = redisConnect("127.0.0.1", 6379);
	kv_handler_t    *handler;

	handler = kv_create(NULL);

	if (c->err) {
		redisFree(c);
		return -1;
	}

	char            strHset[100] = {};
	redisReply      *r;
	redisReply      *r1;
	const char      *command = "keys *";

	r = (redisReply *)redisCommand(c, command);

	if (r->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < r->elements; j++) {
			char strHgetall[100] = "hgetall ";
			printf("%u) %s\n", j, r->element[j]->str);
			char *command1 = strcat(strHgetall, r->element[j]->str);

			r1 = redisCommand(c, command1);
			printf("keys: %s\n", command1);

			if (r1->type == REDIS_REPLY_ARRAY) {
				for (i = 0; i < r1->elements; i++) {
					if (i == 0) {
						strcpy(strHset, r1->element[0]->str);
						strcat(strHset, " ");
					}

					if (i == 1) {
						strcat(strHset, r1->element[1]->str);
					}
				}

				printf("key=>%s\n", r->element[j]->str);
				printf("strHset:%s\n", strHset);
				set_data_to_kv(r->element[j]->str, strHset, handler);
			}
		}
	}

	kv_destroy(handler);
	freeReplyObject(r);
	freeReplyObject(r1);
	redisFree(c);
	return 0;
}

