/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    example_multi_handler.c
 * @detail  Multi handler treat cmd.
 *
 * @author  qianxiaoyuan
 * @date    2015-07-23
 */	


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libkv.h"



void example_cmd(kv_handler_t *handler, const char *cmd, unsigned int cmdlen)
{
	int i;
	unsigned long len;
	kv_answer_t *ans;
	kv_answer_value_t *value;
	kv_answer_iter_t *iter;

	if(!cmd || cmdlen ==0)	{
		fprintf(stderr, "Invalid command,try again!\n\n");
		return;
	}

	fprintf(stdout, "\n===>[%s]\n", cmd);
	
	ans=kv_ask(handler, cmd, cmdlen);
	if(ans->errnum !=ERR_NONE) {
		printf("errnum:%d error:%s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return;
	}

	/**treat as length to value*/
	printf("output length:\n");
	len=kv_answer_length(ans);	
	printf("\t%ld\n", len);

	iter=kv_answer_get_iter(ans, ANSWER_HEAD);
	if(len == 1) {
		/**treat as signal value to string*/
		printf("Single value:\n");
		printf("\tvalue:");
		value=kv_answer_first_value(ans);
		for(i = 0; i < value->ptrlen; i++) {
			printf("%c", ((char*)value->ptr)[i]);
		}
		printf("\n");
	} else if(len > 1) {
		printf("Multi value:\n");
		/**treat as first value to string*/
		printf("first value:\n");
		value=kv_answer_first_value(ans);
		printf("\tvalue:%s\n", kv_answer_value_to_string(value));
	
		/**treat as last value to string*/
		printf("last value:\n");
		value=kv_answer_last_value(ans);
		printf("\tvalue:%s\n", kv_answer_value_to_string(value));

		/**treat as multi value to string*/
		
		kv_answer_rewind_iter(ans, iter);
		printf("\tvalues:");
		while((value=kv_answer_next(iter)) != NULL) {
			for(i = 0; i < value->ptrlen; i++) {
				printf("%c", ((char*)value->ptr)[i]);
			}
			printf("  ");
		}
		printf("\n");
	
	} else {
		printf("no value return\n");
	}

	kv_answer_release_iter(iter);
	kv_answer_release(ans);
}		

void example_multi_handler()
{
	printf("\nused_memory before:%d\n", kv_get_used_memory());

	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_handler_t *handlerC;
		

	assert(handlerA=kv_create(NULL));
	assert(handlerB=kv_create(NULL));
	assert(handlerC=kv_create(NULL));

	example_cmd(handlerA,"set key 1", strlen("set key 1"));
	example_cmd(handlerB,"get key", strlen("get key"));
	example_cmd(handlerC,"lpush lkey a bb ccc", strlen("lpush lkey a bb ccc"));
	example_cmd(handlerC,"lrange lkey 0 2", strlen("lrange lkey 0 2"));
	example_cmd(handlerC,"flushdb", strlen("flushdb"));
	example_cmd(handlerC,"dbsize", strlen("dbsize"));

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	kv_destroy(handlerC);

	printf("\nused_memory after:%d\n\n", kv_get_used_memory());
}


int main(int argc, char **argv)
{
	example_multi_handler();	

	return 0;
}
