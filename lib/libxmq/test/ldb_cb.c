#include <stdlib.h>

#include "ldb_cb.h"
#include "slog/slog.h"

void *ldb_pvt_create(const char *path)
{
	char *error = NULL;

	ldb_pvt_t *pvt = (ldb_pvt_t *)calloc(1, sizeof(ldb_pvt_t));

	if (pvt) {
		pvt->options = leveldb_options_create();

		if (!pvt->options) {
			goto CREATE_FAIL;
		}

		leveldb_options_set_create_if_missing(pvt->options, 1);

		pvt->roptions = leveldb_readoptions_create();

		if (!pvt->roptions) {
			goto CREATE_FAIL;
		}

		leveldb_readoptions_set_verify_checksums(pvt->roptions, 1);

		pvt->woptions = leveldb_writeoptions_create();

		if (!pvt->woptions) {
			goto CREATE_FAIL;
		}

		leveldb_writeoptions_set_sync(pvt->woptions, 0);

		pvt->db = leveldb_open(pvt->options, path, &error);

		if (!pvt->db) {
			x_printf(F, "leveldb_open(%s) fail. Error-%s.", path, error);
			leveldb_free(error);
			goto CREATE_FAIL;
		}
	}

	return (void *)pvt;

CREATE_FAIL:
	ldb_pvt_destroy(pvt);

	return NULL;
}

int driver_ldb_put(void *ldb_pvt, const bin_entry_t *keys, const bin_entry_t *values, int pairs)
{
	char            *error = NULL;
	ldb_pvt_t       *pvt = (ldb_pvt_t *)ldb_pvt;

	if (1 == pairs) {
		leveldb_put(pvt->db, pvt->woptions,
			(const char *)(keys->data), (size_t)(keys->len),
			(const char *)(values->data), (size_t)(values->len),
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
			x_printf(E, "leveldb_writebatch_create: Execute fail.");
			return -1;
		}

		int i = 0;

		for (; i < pairs; i++) {
			leveldb_writebatch_put(wbatch,
				(const char *)((keys + i)->data), (size_t)((keys + i)->len),
				(const char *)((values + i)->data), (size_t)((values + i)->len));
		}

		leveldb_write(pvt->db, pvt->woptions, wbatch, &error);

		if (error) {
			x_printf(E, "leveldb_write for batch fail. Error: %s", error);
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

int driver_ldb_get(void *ldb_pvt, const bin_entry_t *keys, bin_entry_t *values, int count)
{
	char            *error = NULL;
	ldb_pvt_t       *pvt = (ldb_pvt_t *)ldb_pvt;

	int i;

	for (i = 0; i < count; i++) {
		values[i].data = leveldb_get(pvt->db, pvt->roptions,
				(const char *)(keys[i].data), (size_t)(keys[i].len),
				(size_t *)&values[i].len, &error);

		if (error) {
			x_printf(E, "leveldb_get error: %s", error);
			leveldb_free(error);
			goto GET_FAIL;
		}
	}

	return 0;

GET_FAIL:

	for (i = 0; i < count; i++) {
		if (values[i].data) {
			leveldb_free(values[i].data);
		}
	}

	return -1;
}

int ldb_pvt_destroy(void *ldb_pvt)
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

