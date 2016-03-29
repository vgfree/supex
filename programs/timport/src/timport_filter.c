#include <stddef.h>

#include "redis_parse.h"
#include "hashmap.h"

#include "misc.h"
#include "timport_filter.h"

#define TSDB_SIZE       8192
#define ACCOUNT_SIZE    10
#define BIZID_SIZE      34
#define UNKNOW_SIZE     6
#define IMEI_SIZE       15
#define TIMESTAMP_SIZE  10

#define IS_UNKNOW(str)  (((*(uint64_t *)(str)) & 0x00ffffffffffffffLL) == *(uint64_t *)"unknow:")

#define IS_IMEI(str)    ((str)[IMEI_SIZE] == ':')

/* redis filters */
int account_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num);

int common_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num);

int geo_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num);

int key_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num);

/* key filters */
int timestamp_key_filter(timport_key_t *p_tkey, char **dst, char *src, int src_len);

int simflow_key_filter(timport_key_t *p_tkey, char **dst, char *src, int src_len);

/* hash filters */
int imei_hash_filter(timport_key_t *p_tkey, char *data, int len);

int account_hash_filter(timport_key_t *p_tkey, char *data, int len);

int common_hash_filter(timport_key_t *p_tkey, char *data, int len);

int geo_hash_filter(timport_key_t *p_tkey, char *data, int len);

int key_hash_filter(timport_key_t *p_tkey, char *data, int len);

/* result filters */
void unique_result_filter(timport_key_t *p_tkey, struct redis_reply **reply);

#define FILTER_DEFINE(x) {#x, (void *)(x) }

struct timport_filter_func
{
	char    *name;
	void    *func;
};

static struct timport_filter_func s_filter_list[] = {
	FILTER_DEFINE(account_redis_filter),
	FILTER_DEFINE(common_redis_filter),
	FILTER_DEFINE(geo_redis_filter),
	FILTER_DEFINE(key_redis_filter),
	FILTER_DEFINE(timestamp_key_filter),
	FILTER_DEFINE(simflow_key_filter),
	FILTER_DEFINE(imei_hash_filter),
	FILTER_DEFINE(account_hash_filter),
	FILTER_DEFINE(common_hash_filter),
	FILTER_DEFINE(geo_hash_filter),
	FILTER_DEFINE(key_hash_filter),
	FILTER_DEFINE(unique_result_filter),
};

static hashmap_t *s_filter_map = NULL;

/* redis filters */
int account_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num)
{
	if ((NULL == data) || (len <= 0)) {
		return -1;
	}

	int     data_len = 0;
	int     offset = IS_UNKNOW(data) ? (UNKNOW_SIZE + 1) : 0;

	if ((offset == 0) && IS_IMEI(data)) {
		data_len = IMEI_SIZE;
	} else if (offset > 0) {
		char *k = NULL;

		for (k = data + offset; k < data + len; ++k) {
			if ((*k == ':') || (*k == '\0')) {
				break;
			}

			data_len++;
		}
	} else {
		data_len = ACCOUNT_SIZE;
	}

	return (int)custom_hash_dist((void *)(data + offset), (unsigned int)data_len, srv_num, 0);
}

int common_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num)
{
	return (int)custom_hash_dist((void *)data, (unsigned int)len, srv_num, 0);
}

int geo_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num)
{
	char    buf[64] = { 0 };
	int     Long = 0;
	int     Lat = 0;
	char    *p = NULL;

	strncpy(buf, data, sizeof(buf));

	if (NULL == (p = strchr(buf, '&'))) {
		return (unsigned int)(-1);
	}

	*p = '\0';

	Long = atoi(buf);
	Lat = atoi(p + 1);

	sprintf(buf, "%d", Long + Lat);

	return (int)custom_hash_dist(buf, strlen(buf), srv_num, 0);
}

int key_redis_filter(timport_key_t *p_tkey, char *data, int len, int srv_num)
{
	char    *dst = NULL;
	int     dst_len = 0;

	if (NULL == p_tkey->key_filter) {
		return -1;
	}

	dst_len = p_tkey->key_filter(p_tkey, &dst, data, len);

	if ((NULL == dst) || (dst_len <= 0)) {
		return -1;
	}

	return (int)custom_hash_dist(dst, dst_len, srv_num, 0);
}

/* key filters */
int timestamp_key_filter(timport_key_t *p_tkey, char **dst, char *src, int src_len)
{
	if (src_len <= TIMESTAMP_SIZE + 1) {
		return -1;
	}

	*dst = src + (TIMESTAMP_SIZE + 1);

	return BIZID_SIZE;
}

int simflow_key_filter(timport_key_t *p_tkey, char **dst, char *src, int src_len)
{
	if (src_len <= (8 + 11)) {
		return -1;
	}

	*dst = src + 8;

	return src_len - (8 + 11);
}

/* hash filters */
int imei_hash_filter(timport_key_t *p_tkey, char *data, int len)
{
	if (len != 15) {
		return -1;
	}

	long long imei = atoll(data);

	return (int)(imei % TSDB_SIZE);
}

int account_hash_filter(timport_key_t *p_tkey, char *data, int len)
{
	if ((NULL == data) || (len <= 0)) {
		return -1;
	}

	int     data_len = 0;
	int     offset = IS_UNKNOW(data) ? (UNKNOW_SIZE + 1) : 0;

	if ((offset == 0) && IS_IMEI(data)) {
		data_len = IMEI_SIZE;
	} else if (offset > 0) {
		char *k = NULL;

		for (k = data + offset; k < data + len; ++k) {
			if ((*k == ':') || (*k == '\0')) {
				break;
			}

			data_len++;
		}
	} else {
		data_len = ACCOUNT_SIZE;
	}

	return (int)custom_hash_dist((void *)(data + offset), (unsigned int)data_len, TSDB_SIZE, 0);
}

int common_hash_filter(timport_key_t *p_tkey, char *data, int len)
{
	return (int)custom_hash_dist((void *)data, (unsigned int)len, TSDB_SIZE, 0);
}

int geo_hash_filter(timport_key_t *p_tkey, char *data, int len)
{
	char    buf[64] = { 0 };
	int     Long = 0;
	int     Lat = 0;
	char    *p = NULL;

	strncpy(buf, data, sizeof(buf));

	if (NULL == (p = strchr(buf, '&'))) {
		return (unsigned int)(-1);
	}

	*p = '\0';

	Long = atoi(buf);
	Lat = atoi(p + 1);

	sprintf(buf, "%d", Long + Lat);

	return (int)custom_hash_dist(buf, strlen(buf), TSDB_SIZE, 0);
}

int key_hash_filter(timport_key_t *p_tkey, char *data, int len)
{
	char    *dst = NULL;
	int     dst_len = 0;

	if (NULL == p_tkey->key_filter) {
		return -1;
	}

	dst_len = p_tkey->key_filter(p_tkey, &dst, data, len);

	if ((NULL == dst) || (dst_len <= 0)) {
		return -1;
	}

	return (int)custom_hash_dist(dst, dst_len, TSDB_SIZE, 0);
}

/* result filters */
void unique_result_filter(timport_key_t *p_tkey, struct redis_reply **reply)
{
	struct redis_reply      *r = *reply;
	int                     elements = 0;
	int                     i = 0;

	elements = unique_array(r);

	if (elements < (int)r->elements) {
		for (i = elements; i < (int)r->elements; ++i) {
			free(r->element[i]->str);
			free(r->element[i]);
			r->element[i] = NULL;
		}

		r->elements = (size_t)elements;
	}
}

/* filter module init */
static void _timport_filter_init(void)
{
	int i = 0;

	s_filter_map = hashmap_open();

	if (NULL == s_filter_map) {
		fprintf(stderr, "[%s:%d] hashmap_open failed!\n", __FUNCTION__, __LINE__);
		exit(EXIT_FAILURE);
		return;
	}

	for (i = 0; i < sizeof(s_filter_list) / sizeof(s_filter_list[0]); ++i) {
		hashmap_set(s_filter_map, (void *)s_filter_list[i].name, (size_t)strlen(s_filter_list[i].name), (void *)&(s_filter_list[i].func), sizeof(void *));
	}
}

void *timport_filter_lookup(const char *filter_name)
{
	void    *func = NULL;
	size_t  len = sizeof(void *);

	if (NULL == s_filter_map) {
		_timport_filter_init();
	}

	hashmap_get(s_filter_map, (void *)filter_name, (size_t)strlen(filter_name), (void *)&func, &len);

	return func;
}

