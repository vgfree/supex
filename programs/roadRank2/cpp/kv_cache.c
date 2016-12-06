#include <assert.h>

#include "kv_cache.h"
#include "custom_hash.h"
#include "libevcs.h"

#include <stdio.h>

kv_cache *kv_cache_create(int kv_num)
{
	if ((kv_num <= 1) || (kv_num > CACHE_POOL_MAX_KV)) {
		x_printf(E, "kv_num should between 1 and %d!\n", kv_num);
		return NULL;
	}

	kv_cache *p_cache = (kv_cache *)malloc(sizeof(kv_cache));
	assert(p_cache);

	p_cache->kv_count = kv_num;
	p_cache->p_kv_buff = (int *)malloc(sizeof(int) * kv_num);

	assert(p_cache->p_kv_buff);
	/*init kv handlers*/

	kv_init();

	struct kv_config cfg = {
                .open_on_disk   = false,
                .time_to_stay   = -1,
        };
	int i;
	for (i = 0; i < kv_num; i++) {
		char name[32] = {0};
		snprintf(name, sizeof(name), "kv-%d", i);

		int ret = kv_load(&cfg, name);
		if (-1 == ret) {
			x_printf(F, "kv_load failed");
			return NULL;
		}
		*(p_cache->p_kv_buff + i) = ret;
	}

	return p_cache;
}

kv_handler_t *kv_cache_spl(kv_cache *p_cache, char *hash_key, char *kv_buff)
{
	if (!p_cache || !p_cache->p_kv_buff || !hash_key || !kv_buff) {
		return NULL;
	}

	int hash_idx = custom_hash_dist(hash_key, p_cache->kv_count, 0);

	if ((hash_idx < 0) || (hash_idx >= p_cache->kv_count)) {
		return NULL;
	}

	int dbidx = *(p_cache->p_kv_buff + hash_idx);
	kv_handler_t *handler = kv_spl(dbidx, kv_buff, strlen(kv_buff));

	return handler;
}

void kv_cache_destory(kv_cache *p_cache)
{
	if (!p_cache) {
		return;
	}

	if (p_cache->p_kv_buff && (p_cache->kv_count > 0)) {
		kv_destroy();
		free(p_cache->p_kv_buff);
	}
	free(p_cache);
}

