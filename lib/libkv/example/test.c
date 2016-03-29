#include <stdio.h>
#include <string.h>

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

int main()
{
	kv_handler_t * handler = NULL;
	handler = kv_create(NULL);
	
	
	example_cmd(handler, "hmset key a 3 b 4", strlen("hmset key a 3 b 4"));
	example_cmd(handler, "hset key b 4", strlen("hset key b 4"));
	example_cmd(handler, "hset key c 5", strlen("hset key c 4"));
	example_cmd(handler, "hset key d 6", strlen("hset key d 6"));
	
	kv_answer_t *ans = NULL;
	kv_answer_value_t *value = NULL;
	kv_answer_iter_t *iter = NULL;
	int i = 0;
	
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	unsigned long len = kv_answer_length(ans);
	if (len != 1){
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);
		while((value = kv_answer_next(iter)) != NULL){
			printf("%s", (char *)value->ptr);
			//for(; i < value->ptrlen; i++){
			//	printf("%c\n", ((char *)value->ptr)[i]);
			//}
		}
	}
	
	kv_answer_release_iter(iter);

	return 0;
}
