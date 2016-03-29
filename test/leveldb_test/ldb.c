/*
 * test performace of level DB
 * auth: coanor
 * date: Wed Sep 25 10:34:58 CST 2013
 */
#include "leveldb/c.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#define KNRM                    "\x1B[0m"
#define KRED                    "\x1B[31m"
#define KGRN                    "\x1B[32m"
#define KYEL                    "\x1B[33m"
#define KBLU                    "\x1B[34m"
#define KMAG                    "\x1B[35m"
#define KCYN                    "\x1B[36m"
#define KWHT                    "\x1B[37m"

#ifndef RANGE_GET_KEY_CNT
  #define RANGE_GET_KEY_CNT     0
#endif

enum ldb_opt
{
	ldb_true = 1,
	ldb_false = 0,
};

struct kv_
{
	char *key, *val;
};

struct kv_list_
{
	struct kv_      *data;
	int             data_cnt;
	char            *file;
	FILE            *fp;
	size_t          header_size;
	long            klen,
		vlen;
};

struct ldb_data_
{
	size_t          kv_list_cnt;
	size_t          kv_cnt;
	size_t          byte_cnt;
	int             load_data_time, write_time, random_get_time, iter_get_time;
	size_t          random_get_byte_cnt;
	size_t          iter_get_byte_cnt;
	size_t          random_get_cnt;
	size_t          random_get_fail_cnt;
	struct kv_list_ *ldb_data;
};

struct ldb_data_ *g_ldb_data = NULL;

static int parse_kv(const char *buf, struct kv_ *kv);

static int read_header(struct kv_list_ *kvl);

static int read_dir(const char *dir);

static void free_big_data(void);

void load_data(void)
{
	time_t  begin = time(NULL);
	int     i = 0;
	long    data_cnt = 0;
	int     ret = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;

		if (kvl == NULL) {
			continue;
		}

		if (read_header(kvl) != 0) {
			continue;
		}

		printf("loading %s...\n", kvl->file);

		/* read data file */
		int     buf_size = kvl->klen + kvl->vlen + 2;	/* append `,` and `;` */
		char    *read_buf = calloc(buf_size + 1, 1);
		kvl->data = calloc(sizeof(struct kv_), kvl->data_cnt);

		int ret = 0;

		int idx = 0;

		while ((ret = fread(read_buf, 1, buf_size, kvl->fp)) > 0) {
			parse_kv(read_buf, &kvl->data[idx]);
			assert(kvl->data[idx].key != NULL);

#if (TEST_RANDOM_READ || TEST_ITER_READ)
			/* do not check value */
#else
			assert(kvl->data[idx].val != NULL);
#endif

			idx++;

			if (idx >= kvl->data_cnt) {
				break;
			}

			g_ldb_data->kv_cnt++;
		}

		free(read_buf);
	}

	g_ldb_data->load_data_time = time(NULL) - begin;

#if (TEST_RANDOM_READ || TEST_ITER_READ)
	printf("%ld Mb data(%ld count key) loaded, time elapsed %d seconds\n",
		(long)g_ldb_data->byte_cnt / 1024 / 1024,
		g_ldb_data->kv_cnt,
		g_ldb_data->load_data_time);
#else
	printf("%ld Mb data(%ld count kv) loaded, time elapsed %d seconds\n",
		(long)g_ldb_data->byte_cnt / 1024 / 1024,
		g_ldb_data->kv_cnt,
		g_ldb_data->load_data_time);
#endif
}

#ifndef BATCH_WRITE_MB_BYTES
  #define BATCH_WRITE_MB_BYTES 512
#endif

static int batch_put_test(leveldb_t *db, int sync)
{
	leveldb_writebatch_t *w_bat = leveldb_writebatch_create();

	leveldb_writeoptions_t *w_opt = leveldb_writeoptions_create();

	leveldb_writeoptions_set_sync(w_opt, sync);

	char            *err = NULL;
	struct kv_      *item = NULL;

	int     i = 0;
	size_t  bat_bytes = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;
		int             j = 0;
		printf("putting %s\n", kvl->file);

		for (; j < kvl->data_cnt; ++j) {
			leveldb_writebatch_put(w_bat,
				kvl->data[j].key, kvl->klen,
				kvl->data[j].val, kvl->vlen);
			/* free value to save memory */
			free(kvl->data[j].val); kvl->data[j].val = NULL;
			bat_bytes += (kvl->klen + kvl->vlen);

			/* try flush some data */
			if (bat_bytes >= 1024 * 1024 * BATCH_WRITE_MB_BYTES) {
				leveldb_write(db, w_opt, w_bat, &err);

				if (err) {
					printf("%s/%d: %sleveldb error: %s\n", __FILE__, __LINE__, KRED, err);
					return -1;
				}

				leveldb_writebatch_clear(w_bat);
				bat_bytes = 0;
			}
		}
	}

	leveldb_writebatch_clear(w_bat);
	leveldb_writebatch_destroy(w_bat);

	return 0;
}

static void put_test(leveldb_t *db, int sync)
{
	/* test write */
	leveldb_writeoptions_t *w_opt = leveldb_writeoptions_create();

	leveldb_writeoptions_set_sync(w_opt, sync);

	struct kv_      *item = NULL;
	char            *err = NULL;

	int i = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;
		int             j = 0;

		for (; j < kvl->data_cnt; ++j) {
			leveldb_put(db, w_opt,
				kvl->data[j].key, kvl->klen,
				kvl->data[j].val, kvl->vlen, &err);

			if (err) {
				printf("%s%s/%d: leveldb error: %s\n", __FILE__, __LINE__, KRED, err);
			}

			/* free value to save memory */
			free(kvl->data[j].val);
			kvl->data[j].val = NULL;
		}
	}

	leveldb_writeoptions_destroy(w_opt);
}

static void ldb_put_test(leveldb_t *db)
{
	time_t begin;

	begin = time(NULL);
#if SYNC_PUT
	printf("==================== SYNC PUT ========================\n");
	put_test(db, ldb_true);
#else
	printf("==================== ASYNC PUT =======================\n");
	put_test(db, ldb_false);
#endif

	printf("data size = %ld Mb\n", (long)g_ldb_data->byte_cnt / 1024 / 1024);
	printf("key count = %ld\n", g_ldb_data->kv_cnt);

	g_ldb_data->write_time = (time(NULL) - begin);

	if (!g_ldb_data->write_time) {
		g_ldb_data->write_time = 1;
	}

	printf("put time elapsed: %d\n",
		g_ldb_data->write_time);
	printf("op/second: %ld\n",
		g_ldb_data->kv_cnt / g_ldb_data->write_time);
	printf("bytes/second: %ld Mb\n",
		g_ldb_data->byte_cnt / 1024 / 1024 / g_ldb_data->write_time);
}

static void ldb_bat_put_test(leveldb_t *db)
{
	time_t begin;

	begin = time(NULL);
#if SYNC_PUT
	printf("==================== BATCH SYNC PUT =======================\n");
	batch_put_test(db, ldb_true);
#else
	printf("==================== BATCH ASYNC PUT =======================\n");
	batch_put_test(db, ldb_false);
#endif
	printf("data size = %ld Mb\n", g_ldb_data->byte_cnt / 1024 / 1024);
	printf("key count = %ld\n", g_ldb_data->kv_cnt);

	g_ldb_data->write_time = (time(NULL) - begin);

	if (!g_ldb_data->write_time) {
		g_ldb_data->write_time = 1;
	}

	printf("put time elapsed: %d\n", g_ldb_data->write_time);
	printf("op/second: %ld\n", g_ldb_data->kv_cnt / g_ldb_data->write_time);
	printf("bytes/second: %ld Mb\n",
		(long)g_ldb_data->byte_cnt / 1024 / 1024 / g_ldb_data->write_time);
}

static void ldb_iter_get_test(leveldb_t *db)
{
	leveldb_readoptions_t *r_opt = leveldb_readoptions_create();

#if READOPTIONS_SET_VERIFY_CHECKSUMS
	leveldb_readoptions_set_verify_checksums(r_opt, ldb_true);
#endif

#if READOPTIONS_SET_FILL_CACHE
	leveldb_readoptions_set_fill_cache(r_opt, ldb_true);
#endif

#if CREATE_SNAPSHOT
	const leveldb_snapshot_t *ss = leveldb_create_snapshot(db);
	leveldb_readoptions_set_snapshot(r_opt, ss);
#endif

	time_t                  begin = time(NULL);
	leveldb_iterator_t      *iter = leveldb_create_iterator(db, r_opt);

	srandom(time(NULL) ^ getpid());
	int     invalid_iter_cnt = 0;
	int     random_iter_cnt = 0;
	int     print_cnt = 0;

	char key_buf[512] = { 0 };

	int i = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;
		int             j = 0;

		while (j < kvl->data_cnt) {
			int             k;
			const char      *iter_key = NULL;
			const char      *iter_val = NULL;
			size_t          len = 0;
			const char      *key = kvl->data[random() % (kvl->data_cnt - 1)].key;
			leveldb_iter_seek(iter, key, kvl->klen);/* set random seek point */

			for (k = 0; k < RANGE_GET_KEY_CNT; ++k) {
				/* check iterator */
				if (!leveldb_iter_valid(iter)) {
					char *iter_err = NULL;
					leveldb_iter_get_error(iter, &iter_err);

					if (iter_err == NULL) {
						invalid_iter_cnt++;
					} else {
						printf("%s%s/%d: leveldb error: %s\n", KRED, __FILE__, __LINE__, iter_err);
					}

					/* drop the iterator, create new one */
					leveldb_iter_destroy(iter);
					iter = leveldb_create_iterator(db, r_opt);
					break;
				}

				leveldb_iter_prev(iter);/* get prev kv */

				/* check iterator */
				if (!leveldb_iter_valid(iter)) {
					char *iter_err = NULL;
					leveldb_iter_get_error(iter, &iter_err);

					if (iter_err == NULL) {
						invalid_iter_cnt++;
					} else {
						printf("%s%s/%d: leveldb error: %s\n", KRED, __FILE__, __LINE__, iter_err);
					}

					/* drop the iterator, create new one */
					leveldb_iter_destroy(iter);
					iter = leveldb_create_iterator(db, r_opt);
					break;
				}

				/* get kv in the position */
				iter_key = leveldb_iter_key(iter, &len);

				if (print_cnt < 10) {
					memcpy(key_buf, iter_key, len);
					key_buf[len] = '\0';
					printf("%s\n", iter_key);
					print_cnt++;
				}

				g_ldb_data->iter_get_byte_cnt += len;
				iter_val = leveldb_iter_value(iter, &len);
				g_ldb_data->iter_get_byte_cnt += len;
			}

			j += k;
			random_iter_cnt++;
		}
	}

	g_ldb_data->iter_get_time = (time(NULL) - begin);

	if (!g_ldb_data->iter_get_time) {
		g_ldb_data->iter_get_time = 1;
	}

	printf("%s==================== ITERATOR GET =======================\n", KYEL);
	printf("%sdata size: %ld Mb\n", KMAG, (long)g_ldb_data->iter_get_byte_cnt / 1024 / 1024);

	printf("%sget time elapsed: %d second\n", KGRN, g_ldb_data->iter_get_time);
	printf("%sget bytes/second: %ld Mb\n", KRED,
		g_ldb_data->iter_get_byte_cnt / g_ldb_data->iter_get_time / 1024 / 1024);

	printf("%srandom cnt: %d\n", KNRM, random_iter_cnt);
	printf("%sinvalid_iter_cnt: %d\n", KBLU, invalid_iter_cnt);

	g_ldb_data->iter_get_byte_cnt = 0;

#if CREATE_SNAPSHOT
	leveldb_release_snapshot(db, ss);
#endif

	leveldb_readoptions_destroy(r_opt);
}

static void ldb_random_get_test(leveldb_t *db)
{
	leveldb_readoptions_t *r_opt = leveldb_readoptions_create();

#if READOPTIONS_SET_VERIFY_CHECKSUMS
	leveldb_readoptions_set_verify_checksums(r_opt, ldb_true);
#endif

#if READOPTIONS_SET_FILL_CACHE
	leveldb_readoptions_set_fill_cache(r_opt, ldb_true);
#endif

#if CREATE_SNAPSHOT
	const leveldb_snapshot_t *ss = leveldb_create_snapshot(db);
	leveldb_readoptions_set_snapshot(r_opt, ss);
#endif

	time_t begin = time(NULL);

	char    *err = NULL;
	char    *val = NULL;
	size_t  vlen = 0;

	srandom(time(NULL) ^ getpid());

	const char      *key = NULL;
	int             i = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;
		int             j = 0;

		for (; j < kvl->data_cnt / 10; ++j) {
			key = kvl->data[random() % (kvl->data_cnt - 1)].key;
			val = leveldb_get(db, r_opt, key, kvl->klen, &vlen, &err);
			g_ldb_data->random_get_cnt++;

			if (err) {
				g_ldb_data->random_get_fail_cnt++;
				printf("%sleveldb error: %s\n", __FILE__, __LINE__, KRED, err);
			} else {
				free(val);
				val = NULL;
				g_ldb_data->random_get_byte_cnt += vlen;
			}
		}
	}

	g_ldb_data->random_get_time = (time(NULL) - begin);

	if (!g_ldb_data->random_get_time) {
		g_ldb_data->random_get_time = 1;
	}

	printf("==================== RANDOM GET =======================\n");
	printf("data size = %ld Mb\n", (long)g_ldb_data->random_get_byte_cnt / 1024 / 1024);
	printf("get count = %ld\n", g_ldb_data->random_get_cnt);
	printf("avg bytes/get = %ld\n",
		g_ldb_data->random_get_byte_cnt / g_ldb_data->random_get_cnt);

	printf("random get time elapsed: %d second\n", g_ldb_data->random_get_time);
	printf("random get op/second: %ld\n",
		g_ldb_data->random_get_cnt / g_ldb_data->random_get_time);
	printf("random get bytes/second: %ld Mb\n",
		g_ldb_data->random_get_byte_cnt / g_ldb_data->random_get_time / 1024 / 1024);

#if CREATE_SNAPSHOT
	leveldb_release_snapshot(db, ss);
#endif

	leveldb_readoptions_destroy(r_opt);
}

int main(int argc, char *argv[])
{
	int ret = 0;

	g_ldb_data = calloc(sizeof(*g_ldb_data), 1);

	if (argc < 2) {
		read_dir(".");
	} else {
		int i = 0;
		g_ldb_data->kv_list_cnt = argc - 1;
		g_ldb_data->ldb_data = calloc(sizeof(struct kv_list_), g_ldb_data->kv_list_cnt);

		for (; i < g_ldb_data->kv_list_cnt; ++i) {
			g_ldb_data->ldb_data[i].file = argv[i + 1];	/* ignore argv[0] */
		}
	}

	assert(g_ldb_data != NULL);
	load_data();

	leveldb_options_t *db_opt = leveldb_options_create();
	leveldb_options_set_create_if_missing(db_opt, ldb_true);
	leveldb_options_set_error_if_exists(db_opt, ldb_false);
	leveldb_options_set_write_buffer_size(db_opt, 1024 * 1024 * 32);/* 32Mb */

	leveldb_cache_t *db_cache = leveldb_cache_create_lru((size_t)8 * 1024 * 1024 * 1024);
	leveldb_options_set_cache(db_opt, db_cache);
	leveldb_options_set_block_size(db_opt, 64 * 1024 * 1024);

#if COMPRESSION
	leveldb_options_set_compression(db_opt, leveldb_snappy_compression);
#endif

	char *ldb_err = NULL;

	leveldb_t *db = leveldb_open(db_opt, "db", &ldb_err);

	if (ldb_err != NULL) {
		printf("%s/%d: leveldb error: %s\n", __FILE__, __LINE__, ldb_err);
		exit(EXIT_FAILURE);
	}

#if BATCH_PUT
	ldb_bat_put_test(db);
#endif

#if DIRECT_PUT
	ldb_put_test(db);
#endif

#if TEST_RANDOM_READ
	ldb_random_get_test(db);
#endif

#if LONG_TIME_READ
	while (1) {
#endif

#if TEST_ITER_READ
	ldb_iter_get_test(db);
#endif

#if LONG_TIME_READ
	sleep(1);
}
#endif

	free_big_data();

	leveldb_close(db);
	leveldb_cache_destroy(db_cache);
	leveldb_options_destroy(db_opt);

	printf("%s==================== DONE ============================\n", KGRN);
	printf("%sis there any leakage?\n", KRED);

	while (1) {
		sleep(1);
	}

	return 0;
}

static int parse_kv(const char *buf, struct kv_ *kv)
{
	if (buf == NULL) {
		return -1;
	}

	char *p = (char *)buf,
	*str_val = NULL,
	*q = NULL;

	q = strchr(p, ',');

	if (q == NULL) {
		return -1;
	}

	kv->key = calloc(q - p + 1, 1);

	if (kv->key == NULL) {
		printf("calloc fail\n");
		exit(EXIT_FAILURE);
	}

	strncpy(kv->key, p, q - p);
	g_ldb_data->byte_cnt += q - p;
#if (TEST_RANDOM_READ || TEST_ITER_READ)
	/* do not parse value */
#else
	p = q + 1;
	q = strchr(p, ';');

	if (q == NULL) {
		return -1;
	}

	kv->val = calloc(q - p + 1, 1);	/* drop trailing `;` */

	if (kv->val == NULL) {
		printf("calloc fail\n");
		exit(EXIT_FAILURE);
	}

	strncpy(kv->val, p, q - p);
	g_ldb_data->byte_cnt += q - p;
#endif

	return 0;
}

static int read_header(struct kv_list_ *kvl)
{
	if (kvl == NULL) {
		return -1;
	}

	char    file_header[512] = { 0 };
	FILE    *fp = fopen(kvl->file, "r");

	if (fp <= 0) {
		return -1;
	}

	kvl->fp = fp;

	if (NULL == fgets(file_header, 512, fp)) {
		return -1;
	}

	/*
	 * ksz,vsz,cnt\n
	 */
	char *p = file_header, *q = NULL;
	kvl->klen = atol(p);
	q = strchr(p, ',');

	if (q == NULL) {
		return -1;
	}

	kvl->vlen = atol(q + 1);

	p = q + 1;
	q = strchr(p, ',');

	if (q == NULL) {
		return -1;
	}

	kvl->data_cnt = atol(q + 1);

	kvl->header_size = strlen(file_header);
	return 0;
}

static int read_dir(const char *dir)
{
	DIR *d = NULL;

	if (dir == NULL) {
		return -1;
	}

	char            *file_list[512] = { NULL };
	struct dirent   *de;
	d = opendir(dir);

	if (d) {
		while ((de = readdir(d)) != NULL) {
			if (strstr(de->d_name, ".ldb") != NULL) {
				file_list[g_ldb_data->kv_list_cnt] = strdup(de->d_name);
				g_ldb_data->kv_list_cnt++;
			} else { continue; }
		}
	}

	closedir(d);

	g_ldb_data->ldb_data = calloc(sizeof(struct kv_list_), g_ldb_data->kv_list_cnt);
	assert(g_ldb_data->ldb_data != NULL);

	int i = 0;

	for (i; i < g_ldb_data->kv_list_cnt; ++i) {
		if (file_list[i] == NULL) {
			break;
		}

		if (i >= 512) {
			printf("%s too much data file(max is 512)\n", KYEL);
			break;
		}

		g_ldb_data->ldb_data[i].file = file_list[i];
	}

	return 0;
}

/* TODO */
static void free_big_data(void)
{
	int i = 0;

	for (; i < g_ldb_data->kv_list_cnt; ++i) {
		struct kv_list_ *kvl = g_ldb_data->ldb_data + i;
		int             j = 0;

		for (; j < kvl->data_cnt; ++j) {
			if (kvl->data[j].key) {
				free(kvl->data[j].key);
				kvl->data[j].key = NULL;
			}

			if (kvl->data[j].val) {
				free(kvl->data[j].val);
				kvl->data[j].val = NULL;
			}
		}

		free(kvl->data); kvl->data = NULL;
		free(kvl->file); kvl->file = NULL;
		fclose(kvl->fp);
	}

	free(g_ldb_data->ldb_data); g_ldb_data->ldb_data = NULL;
	free(g_ldb_data); g_ldb_data = NULL;
}

