/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    example_basic.c
 * @detail  Single handler to output single_value or multi_value.
 *
 * @author  qianxiaoyuan
 * @date    2015-07-23
 */	

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libkv.h"



void example_output_single_value(kv_answer_t *ans)
{
	int i;
	kv_answer_value_t *value;
	
	/**treat as signal value to string*/
	printf("Single value:\n");
	printf("\tvalue:");
	value=kv_answer_first_value(ans);
	for(i = 0; i < value->ptrlen; i++) {
		printf("%c", ((char*)value->ptr)[i]);
	}
	printf("\n");
}

void example_output_multiple_values(kv_answer_t *ans)
{	
	int i;
	kv_answer_value_t *value;
	kv_answer_iter_t *iter;

	/**treat as multi value to string*/
	printf("Multi value:\n");
	iter=kv_answer_get_iter(ans, ANSWER_HEAD);
	kv_answer_rewind_iter(ans, iter);
	printf("\tvalues:");
	while((value=kv_answer_next(iter)) != NULL) {
		for(i = 0; i < value->ptrlen; i++) {
			printf("%c", ((char*)value->ptr)[i]);
		}
		printf("  ");
	}
	printf("\n");

	kv_answer_release_iter(iter);	
}

void example_cmd(kv_handler_t *handler, const char *cmd, unsigned int cmdlen)
{	
	kv_answer_t *ans;

	if(!cmd || cmdlen ==0) {
		fprintf(stderr, "Invalid command, try again!\n\n");
		return;
	}

	ans=kv_ask(handler, cmd, cmdlen);
	if (ans->errnum != ERR_NONE) {
		printf("errnum:%d\terr:%s\n\n", ans->errnum, ans->err);
		return;
	}
       
	fprintf(stdout, "\n===>[%s]\n", cmd);
	unsigned long len=kv_answer_length(ans);
	if(len == 1)
		example_output_single_value(ans);
	else if (len > 1) 
		example_output_multiple_values(ans);
	else
		printf("===>no value returned\n");

	kv_answer_release(ans);
}

void example_basic()
{
	printf("\nused_memory before:%d\n", kv_get_used_memory());

	kv_handler_t *handler;

	handler = kv_create(NULL);
	
	example_cmd(handler,"set key 1", strlen("set key 1"));
	example_cmd(handler,"get key", strlen("get key"));
	example_cmd(handler,"lpush lkey a bb ccc", strlen("lpush lkey a bb ccc"));
	example_cmd(handler,"lrange lkey 0 2", strlen("lrange lkey 0 2"));
	
	kv_destroy(handler);

	printf("\nused_memory after:%d\n\n", kv_get_used_memory());
}



int main(int argc, char **argv)
{
	example_basic();

	return 0;
}
