#include <stdlib.h>

#include "ldb_cb.h"
#include "slog/slog.h"

extern ldb_pvt_t *ldb_pvt_create(const char *path)
{
	char *error = NULL;

	ldb_pvt_t *pvt = (ldb_pvt_t *)calloc(sizeof(ldb_pvt_t), 1);

	if (pvt) {
		pvt->options = leveldb_options_create();

		if (!pvt->options) {
			goto dislike;
		}

		leveldb_options_set_create_if_missing(pvt->options, 1);

		pvt->roptions = leveldb_readoptions_create();

		if (!pvt->roptions) {
			goto dislike;
		}

		leveldb_readoptions_set_verify_checksums(pvt->roptions, 1);

		pvt->woptions = leveldb_writeoptions_create();

		if (!pvt->woptions) {
			goto dislike;
		}

		leveldb_writeoptions_set_sync(pvt->woptions, 0);

		pvt->db = leveldb_open(pvt->options, path, &error);

		if (!pvt->db) {
			goto dislike;
		}
	}

	return pvt;

dislike:

	if (error) {
		leveldb_free(error);
	}

	ldb_pvt_destroy(pvt);
	return NULL;
}

extern int driver_ldb_put(void *ldb_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int pairs)
{
	char            *error = NULL;
	ldb_pvt_t       *pvt = (ldb_pvt_t *)ldb_pvt;

	if (1 == pairs) {
		binary_entry_t *tmp_k = keys, *tmp_v = values;
		leveldb_put(pvt->db, pvt->woptions,
			(const char *)(tmp_k->data), (size_t)(tmp_k->len),
			(const char *)(tmp_v->data), (size_t)(tmp_v->len),
			&error);

		if (error) {
			x_printf(E, "leveldb_put error: %s", error);
			leveldb_free(error);
			return -1;
		}

		return 0;
	} else if (1 <= pairs) {
		leveldb_writebatch_t *wbatch;
		wbatch = leveldb_writebatch_create();

		if (!wbatch) {
			return -1;
		}

		binary_entry_t  *tmp_k, *tmp_v;
		unsigned int    i;

		for (i = 0; i < pairs; i++) {
			tmp_k = keys + i;
			tmp_v = values + i;
			leveldb_writebatch_put(wbatch,
				(const char *)(tmp_k->data), (size_t)(tmp_k->len),
				(const char *)(tmp_v->data), (size_t)(tmp_v->len));
		}

		leveldb_write(pvt->db, pvt->woptions, wbatch, &error);

		if (error) {
			x_printf(E, "leveldb_write error: %s", error);
			leveldb_free(error);
			leveldb_writebatch_clear(wbatch);
			leveldb_writebatch_destroy(wbatch);
			return -1;
		}

		leveldb_writebatch_clear(wbatch);
		leveldb_writebatch_destroy(wbatch);

		return 0;
	}

	return -1;
}

extern int driver_ldb_get(void *ldb_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int count)
{
	char            *error = NULL;
	ldb_pvt_t       *pvt = (ldb_pvt_t *)ldb_pvt;

	unsigned int i;

	for (i = 0; i < count; i++) {
		values[i].data = leveldb_get(pvt->db, pvt->roptions,
				(const char *)(keys[i].data), (size_t)(keys[i].len),
				(size_t *)&values[i].len, &error);

		if (error) {
			x_printf(E, "leveldb_get error: %s", error);
			leveldb_free(error);
			goto dislike;
		}
	}

	return 0;

dislike:

	for (i = 0; i < count; i++) {
		if (values[i].data) {
			free(values[i].data);
		}
	}

	return -1;
}

extern int ldb_pvt_destroy(void *ldb_pvt)
{
	ldb_pvt_t *pvt = (ldb_pvt_t *)ldb_pvt;

	if (pvt) {
		if (pvt->db) {
			leveldb_close(pvt->db);
		}

		if (pvt->woptions) {
			leveldb_writeoptions_destroy(pvt->woptions);
		}

		if (pvt->roptions) {
			leveldb_readoptions_destroy(pvt->roptions);
		}

		if (pvt->options) {
			leveldb_options_destroy(pvt->options);
		}

		free(pvt);
	}

	return 0;
}

