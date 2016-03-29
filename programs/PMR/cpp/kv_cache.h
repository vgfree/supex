#pragma once
#include "libkv.h"

#define CACHE_POOL_MAX_KV 32

typedef struct kv_cache
{
	int             kv_count;	/*kv handler count*/
	kv_handler_t    **p_kv_buff;	/*kv handler buffer*/
} kv_cache;

kv_cache *kv_cache_create(int kv_num);

kv_answer_t *kv_cache_ask(kv_cache *p_cache, char *hash_key, char *kv_buff);

void kv_cache_destory(kv_cache *p_cache);

