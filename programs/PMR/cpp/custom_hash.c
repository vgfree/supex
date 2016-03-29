#include "custom_hash.h"
/*murmur hash*/
unsigned int MurmurHash2(const void *key, int len, unsigned int seed)
{
	const unsigned int      m = 0x5bd1e995;
	const int               r = 24;

	unsigned int h = seed ^ len;

	const unsigned char *data = (const unsigned char *)key;

	while (len >= 4) {
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	switch (len)
	{
		case 3:
			h ^= data[2] << 16;

		case 2:
			h ^= data[1] << 8;

		case 1:
			h ^= data[0];
			h *= m;
	}
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

/**
 * name:custom_hash_dist
 * func:hash customer's account id to distribute customers to @_srv_num servers
 * args:@_account custom's ID  @_srv_num total servers   @_suffix  expand suffix
 * return:fail -1;sucess [1,@_srv_num]
 */
int custom_hash_dist(char *_account, int _srv_num, int _suffix)
{
	unsigned int    hash = 0;
	unsigned int    seed = 7;

	if ((_account == NULL) || (strncmp(_account, "", 1) == 0) || (_srv_num <= 0)) {
		return -1;
	}

	hash = MurmurHash2((const void *)_account, strlen(_account), seed);
	return hash % _srv_num + _suffix;
}

