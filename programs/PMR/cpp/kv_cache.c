#include <assert.h>

#include "kv_cache.h"
#include "custom_hash.h"
#include "utils.h"

#include <stdio.h>

kv_cache *kv_cache_create(int kv_num)
{
	if ((kv_num < 1) || (kv_num > CACHE_POOL_MAX_KV)) {
		x_printf(E, "kv_num should between 1 and %d!\n", kv_num);
		return NULL;
	}

	kv_cache *p_cache = (kv_cache *)malloc(sizeof(kv_cache));
	assert(p_cache);

	p_cache->kv_count = kv_num;
	p_cache->p_kv_buff = (kv_handler_t **)malloc(sizeof(kv_handler_t *) * kv_num);

	assert(p_cache->p_kv_buff);
	/*init kv handlers*/
	int i;

	for (i = 0; i < kv_num; i++) {
		kv_handler_t *p_kv = kv_create(NULL);
		*(p_cache->p_kv_buff + i) = p_kv;
	}

	return p_cache;
}

kv_answer_t *kv_cache_ask(kv_cache *p_cache, char *hash_key, char *kv_buff)
{
	if (!p_cache || !p_cache->p_kv_buff || !hash_key || !kv_buff) {
		return NULL;
	}

	int hash_idx = custom_hash_dist(hash_key, p_cache->kv_count, 0);

	if ((hash_idx < 0) || (hash_idx >= p_cache->kv_count)) {
		return NULL;
	}

	kv_handler_t    *p_kv_handle = *(p_cache->p_kv_buff + hash_idx);
	kv_answer_t     *p_ans = kv_ask(p_kv_handle, kv_buff, strlen(kv_buff));

	return p_ans;
}

void kv_cache_destory(kv_cache *p_cache)
{
	if (!p_cache) {
		return;
	}

	if (p_cache->p_kv_buff && (p_cache->kv_count > 0)) {
		int i;

		for (i = 0; i < p_cache->kv_count; i++) {
			kv_destroy(*(p_cache->p_kv_buff + i));
		}

		free(p_cache->p_kv_buff);
	} else {
		free(p_cache);
	}
}

