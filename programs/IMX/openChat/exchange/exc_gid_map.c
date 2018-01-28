#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libkv.h"
#include "libmini.h"
#include "exc_comm_def.h"
#include "exc_gid_map.h"

static int g_gid_map = 0;

void exc_gidmap_init(void)
{
	g_gid_map = kv_load(NULL, NULL);
}

void exc_gidmap_free(void)
{}

int exc_gidmap_get_cid(char gid[MAX_GID_SIZE], GID_FOR_EACH_CID fcb, void *usr)
{
	char cmd[32 + MAX_GID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "smembers GID_CID:%s", gid);

	kv_handler_t    *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	unsigned long len = answer_length(ans);

	if (len) {
		int                     idx = 0;
		kv_answer_iter_t        *iter = answer_iter_make(ans, ANSWER_HEAD);
		kv_answer_value_t       *value;

		while ((value = answer_iter_next(iter)) != NULL) {
			assert(answer_value_look_type(value) != VALUE_TYPE_NIL);

			char cid[MAX_CID_SIZE] = { 0 };
			memset(cid, 0, sizeof(cid));

			if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
				memcpy(cid, answer_value_look_addr(value), MIN(answer_value_look_size(value), sizeof(cid)));
			} else {
				snprintf(cid, sizeof(cid), "%d", answer_value_look_int(value));
			}

			fcb(cid, idx, usr);
			idx++;
		}

		answer_iter_free(iter);
	}

	kv_handler_release(handler);
	return 0;
}

// FIXME:to fill not fd by of cid
int exc_gidmap_add_cid(char gid[MAX_GID_SIZE], char cid[MAX_CID_SIZE])
{
	char cmd[32 + MAX_GID_SIZE + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "sadd GID_CID:%s %s", gid, cid);

	kv_handler_t    *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

int exc_gidmap_rem_cid(char gid[MAX_GID_SIZE], char cid[MAX_CID_SIZE])
{
	char cmd[32 + MAX_GID_SIZE + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "srem GID_CID:%s %s", gid, cid);

	kv_handler_t    *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

