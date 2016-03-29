/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    example_multi_thread.c
 * @detail  Single thread to handle single handler, multi thread to handle single handler.
 *
 * @author  qianxiaoyuan
 * @date    2015-07-23
 */	

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "libkv.h"



typedef void*(*thread_handle)(void*);

struct thread
{
	thread_handle hd;
	pthread_t thread1,thread2,thread3;
	const char *thread1_name, *thread2_name, *thread3_name;
	pthread_mutex_t lock;
};

typedef struct thread *pthr_t;
pthr_t p;


void example_cmd(kv_handler_t *handler, const char *cmd, unsigned int cmdlen)
{

	int i;
	kv_answer_t *ans;
	kv_answer_value_t *value;
	kv_answer_iter_t *iter;

	
	printf("\n===>[%s->%s]\n", pthread_equal(pthread_self(), p->thread1) ? p->thread1_name : 
			   	   pthread_equal(pthread_self(), p->thread2) ? p->thread2_name :
			           pthread_equal(pthread_self(), p->thread3) ? p->thread3_name : "Thread can't be tracked!",
			           cmd);

	if(!cmd || cmdlen ==0) {
		fprintf(stderr,"Invalid command,try again!\n\n");
		return;
	}
	
	ans=kv_ask(handler, cmd, cmdlen);
	if(ans->errnum != ERR_NONE) {
		printf("errnum:%d\terror:%s\n\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return;
	}
	
	unsigned long len=kv_answer_length(ans);
	printf("output length:\n\t%ld\n", len);

	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
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
		printf("no value returned\n");
	}

	kv_answer_release_iter(iter);
	kv_answer_release(ans);
}
void *example_handle(void *arg)
{
	pthread_mutex_lock(&p->lock);
	
	kv_handler_t *handler=(kv_handler_t *)arg;

	example_cmd(handler, "set key value_to_test", strlen("set key value_to_test"));
	example_cmd(handler, "get key", strlen("get key"));
	example_cmd(handler, "lpush lkey a bb ccc", strlen("lpush lkey a bb ccc"));
	example_cmd(handler, "lrange lkey 0 2", strlen("lrange lkey 0 2"));
	example_cmd(handler, "flushdb", strlen("flushdb"));
	example_cmd(handler, "dbsize", strlen("dbsize"));
		
	pthread_mutex_unlock(&p->lock);
	
	return NULL;
	
}

void example_multi_threads()
{
	printf("\nused_memory before:%d\n", kv_get_used_memory());
	p=(pthr_t)malloc(sizeof(struct thread));

	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	
	p->hd=example_handle;
	pthread_mutex_init(&p->lock, NULL);

	p->thread1_name = "ThreadA->handlerA";
	p->thread2_name = "ThreadB->handlerB";
	p->thread3_name = "ThreadC->handlerB";
	pthread_create(&p->thread1, NULL, p->hd, (void*)handlerA);
	pthread_create(&p->thread2, NULL, p->hd, (void*)handlerB);
	pthread_create(&p->thread3, NULL, p->hd, (void*)handlerB);	

	pthread_join(p->thread1, NULL);
	pthread_join(p->thread2, NULL);
	pthread_join(p->thread3, NULL);

	pthread_mutex_destroy(&p->lock);

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	
	free(p);
	printf("\nused_memory after:%d\n\n", kv_get_used_memory());
}



int main(int argc, char **argv)
{
	example_multi_threads();	

	return 0;
}
