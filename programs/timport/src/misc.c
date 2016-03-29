#include <string.h>

#include "misc.h"

#define MAX_CMD_SIZE    256

#define TIMESTAMP_SIZE  10

extern struct redis_reply       *redis_reply_create();

int64_t get_stime(int32_t fd)
{
	char    buf[16] = { 0 };
	ssize_t size = pread(fd, buf, 16, 0);

	if (size == -1) {
		printf("read start time error, errno:[%d].\n", errno);
		raise(SIGQUIT);
	}

	/* check start time. */
	uint64_t st = atoll(buf);

	if ((st < 1000000000) || (st > 2000000000)) {
		printf("start time:[%ld] is error.\n", st);
		raise(SIGQUIT);
	}

	return st;
}

int32_t set_stime(int32_t fd, int64_t t)
{
	char buf[16] = { 0 };

	sprintf(buf, "%ld", t);
	ssize_t size = pwrite(fd, buf, strlen(buf), 0);

	if (size != 10) {
		printf("write start time is error, errno:[%d].\n", errno);
		raise(SIGQUIT);
	}

	return 0;
}

/* get file size. */
inline int64_t get_file_size(const char *path)
{
	int64_t         file_size = -1;
	struct stat     statbuff;

	if (stat(path, &statbuff) < 0) {
		return file_size;
	} else {
		file_size = statbuff.st_size;
		return file_size;
	}
}

/* write data to file. */
inline int write_file(int32_t fd, const char *buf, int32_t count)
{
	int32_t write_size = 0;

	while (count > 0) {
		write_size = write(fd, buf, count);

		if (write_size < 0) {
			return -1;
		}

		buf += write_size;
		count -= write_size;
	}

	return 0;
}

static int compare_elementp(const void *p1, const void *p2)
{
	return strcmp((*(struct redis_reply *const *)p1)->str, (*(struct redis_reply *const *)p2)->str);
}

int sort_array(struct redis_reply *reply)
{
	qsort(reply->element, reply->elements, sizeof(reply->element[0]), compare_elementp);
	return 0;
}

/* ONLY USED IN RGPS */
int unique_array(struct redis_reply *reply)
{
	int i, j;

	for (i = 0, j = 1; j < reply->elements; ) {
		int cmp = strncmp(reply->element[i]->str, reply->element[j]->str, TIMESTAMP_SIZE);

		if (cmp == 0) {
			j++;
		} else if (cmp < 0) {
			i++;

			if (i != j) {
				struct redis_reply *tmp = reply->element[i];
				reply->element[i] = reply->element[j];
				reply->element[j] = tmp;
			}

			j++;
		} else {
			/* WARNNING: unsorted array */
			return reply->elements;
		}
	}

	return i + 1;
}

/* common function. */
inline int get_column_count(const char *row, char sep)
{
	int cols = 0;

	if ((row == NULL) || (*row == sep)) {
		return cols;
	}

	while (row != NULL && *row != 0) {
		++cols;
		++row;
		row = strchr(row, sep);
	}

	return cols;
}

int set_to_string(struct redis_reply **dst, struct redis_reply *src, int ignore_error)
{
	int                     i = 0;
	int                     cols = 0;
	int                     total_len = 32;
	struct redis_reply      *reply_str = NULL;

	if ((NULL == dst) || (NULL == src) || (REDIS_REPLY_ARRAY != src->type) || (0 == src->elements)) {
		x_printf(F, "param invalid!");
		return -1;
	}

	*dst = NULL;

	reply_str = redis_reply_create();

	if (NULL == reply_str) {
		x_printf(F, "redis_reply_create failed!");
		return -1;
	}

	reply_str->type = REDIS_REPLY_STRING;

	for (i = 0; i < (int)src->elements; ++i) {
		total_len += src->element[i]->len + 1;
	}

	reply_str->str = calloc(1, total_len);

	/* concat array's data to a string. */
	cols = get_column_count(src->element[0]->str, '|');

	if (cols == 0) {
		x_printf(F, "get_column_count == 0");
		redis_reply_release(reply_str);
		return -1;
	}

	sprintf(reply_str->str, "%ld*%d@", (long)src->elements, cols);
	reply_str->len = strlen(reply_str->str);

	for (i = 0; i < (int)src->elements; ++i) {
		/* check buffer size. */
		if (reply_str->len + src->element[i]->len >= total_len) {
			x_printf(F, "data size [%d] out of range\n", reply_str->len);
			redis_reply_release(reply_str);
			return -1;
		}

		/* check column count. */
		if (cols != get_column_count(src->element[i]->str, '|')) {
			x_printf(F, "column count [%d] error, row [%s]\n", get_column_count(src->element[i]->str, '|'), src->element[i]->str);

			if (!ignore_error) {
				redis_reply_release(reply_str);
				return -1;
			}
		}

		/* copy data. */
		memcpy(reply_str->str + reply_str->len, src->element[i]->str, src->element[i]->len);
		reply_str->len += src->element[i]->len;
		reply_str->str[reply_str->len++] = '|';
	}

	*dst = reply_str;
	return 0;
}

// TODO: not thread safe
char *get_ftime(time_t tmstamp, tkey_interval_e interval)
{
	static char     ten_min_ftime[32] = { 0 };
	static char     one_hour_ftime[32] = { 0 };

	char *p = ten_min_ftime;

	switch (interval)
	{
		case TKEY_INTERVAL_TEN_MIN:
			p = ten_min_ftime;
			strftime(ten_min_ftime, sizeof(ten_min_ftime), "%Y%m%d%H%M", localtime(&tmstamp));
			ten_min_ftime[strlen(ten_min_ftime) - 1] = 0;
			break;

		case TKEY_INTERVAL_ONE_HOUR:
			p = one_hour_ftime;
			strftime(one_hour_ftime, sizeof(one_hour_ftime), "%Y%m%d%H", localtime(&tmstamp));
			break;

		default:
			return NULL;
	}

	return p;
}

typedef enum
{
	CMD_TYPE_SET,
	CMD_TYPE_GET,
	CMD_TYPE_EXPIRE
} cmd_type_e;

int fmt_task_key(timport_task_t *p_ttask)
{
	timport_key_t   *p_tkey = NULL;
	char            *dst = NULL;
	int             dst_len = 0;
	int             len = 0;

	if ((NULL == p_ttask) || (NULL == p_ttask->tkey)) {
		return -1;
	}

	dst = p_ttask->param;
	dst_len = p_ttask->param_len;

	p_ttask->key[0] = '\0';

	p_tkey = p_ttask->tkey;

	if (p_tkey->param_cnt == 1) {
		if (NULL == p_ttask->ftime) {
			return -1;
		} else {
			if (NULL == p_ttask->param) {
				sprintf(p_ttask->key, "%s:%s", p_tkey->key, p_ttask->ftime);
			} else {
				len = sprintf(p_ttask->key, "%s:", p_tkey->key);

				if (p_tkey->key_filter) {
					dst_len = p_tkey->key_filter(p_ttask->tkey, &dst, p_ttask->param, p_ttask->param_len);
				}

				memcpy(p_ttask->key + len, dst, dst_len);
				len += dst_len;
				p_ttask->key[len] = '\0';
			}
		}
	} else if ((p_tkey->param_cnt == 2) && (NULL != p_ttask->param) && (0 < p_ttask->param_len)) {
		len = sprintf(p_ttask->key, "%s:", p_tkey->key);

		if (p_tkey->param_tm_pos == 0) {
			strcat(p_ttask->key, p_ttask->ftime);
			strcat(p_ttask->key, ":");
			len += strlen(p_ttask->ftime) + 1;

			if (p_tkey->key_filter) {
				dst_len = p_tkey->key_filter(p_ttask->tkey, &dst, p_ttask->param, p_ttask->param_len);
			}

			memcpy(p_ttask->key + len, dst, dst_len);
			len += dst_len;
			p_ttask->key[len] = '\0';
		} else {
			if (p_tkey->key_filter) {
				dst_len = p_tkey->key_filter(p_ttask->tkey, &dst, p_ttask->param, p_ttask->param_len);
			}

			memcpy(p_ttask->key + len, dst, dst_len);
			len += dst_len;
			p_ttask->key[len] = '\0';
			strcat(p_ttask->key, ":");
			strcat(p_ttask->key, p_ttask->ftime);
		}
	} else {
		return -1;
	}

	return 0;
}

int merge_child_sets(timport_task_t *p_ttask)
{
	struct redis_reply      *reply = NULL;
	timport_task_t          *p = NULL;
	timport_task_t          *q = NULL;
	int                     i = 0;
	int                     j = 0;

	if ((NULL == p_ttask) || (NULL == p_ttask->child)) {
		return -1;
	}

	reply = redis_reply_create();

	if (NULL == reply) {
		return -1;
	}

	reply->type = REDIS_REPLY_ARRAY;
	reply->elements = 0;

	p = p_ttask->child;

	while (NULL != p) {
		if ((NULL != p->reply) && (REDIS_REPLY_ARRAY == p->reply->type)) {
			reply->elements += p->reply->elements;
		}

		p = p->slibing;
	}

	if (0 != reply->elements) {
		NewArray0(reply->elements, reply->element);

		if (NULL == reply->element) {
			free(reply);
			return -1;
		}
	}

	p = p_ttask->child;

	while (NULL != p) {
		if ((NULL != p->reply) && (REDIS_REPLY_ARRAY == p->reply->type)) {
			for (j = 0; j < (int)p->reply->elements; ++j) {
				reply->element[i + j] = p->reply->element[j];
			}

			i += (int)p->reply->elements;
			free(p->reply->element);
		}

		q = p;
		p = p->slibing;

		if (NULL != q->reply) {
			free(q->reply);
		}

		free(q);
	}

	if (0 == reply->elements) {
		free(reply);
		reply = NULL;
	}

	p_ttask->child = NULL;
	p_ttask->reply = reply;

	return 0;
}

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
unsigned int custom_hash_dist(const void *data, unsigned int len, int srv_num, int suffix)
{
	unsigned int    hash = 0;
	unsigned int    seed = 7;

	if ((data == NULL) || (srv_num <= 0)) {
		return -1;
	}

	hash = MurmurHash2((const void *)data, len, seed);

	return (hash % srv_num) + suffix;
}

struct redis_reply *redis_reply_dup(struct redis_reply *src)
{
	struct redis_reply      *r = NULL;
	size_t                  i = 0;

	if (NULL == src) {
		return NULL;
	}

	r = redis_reply_create();

	if (NULL == r) {
		return NULL;
	}

	switch (src->type)
	{
		case REDIS_REPLY_INTEGER:
			r->integer = src->integer;

		case REDIS_REPLY_NIL:
			r->type = src->type;
			break;

		case REDIS_REPLY_ERROR:
			r->err = src->err;

		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_STRING:
			r->len = src->len;
			r->str = x_strdup(src->str);
			r->type = src->type;
			break;

		case REDIS_REPLY_ARRAY:
			r->elements = src->elements;

			if (src->elements > 0) {
				NewArray0(src->elements, r->element);

				for (i = 0; i < src->elements; ++i) {
					r->element[i] = redis_reply_dup(src->element[i]);
				}
			}

			r->type = src->type;
			break;

		default:

			// FIXME: redis_parser.c bug
			if ((src->len > 0) && (src->str != NULL)) {
				r->type = REDIS_REPLY_STRING;
				r->len = src->len;
				r->str = x_strdup(src->str);
			} else {
				free(r);
				return NULL;
			}

			break;
	}

	return r;
}

