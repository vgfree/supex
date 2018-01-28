#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libkv.h"
#include "libmini.h"
#include "exc_cid_map.h"

static int g_cid_map = 0;

void exc_cidmap_init(void)
{
	g_cid_map = kv_load(NULL, NULL);
}

void exc_cidmap_free(void)
{}

int exc_cidmap_set_uid(char cid[MAX_CID_SIZE], char uid[MAX_UID_SIZE])
{
	char cmd[32 + MAX_UID_SIZE + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "set CID_UID:%s %s", cid, uid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

int exc_cidmap_get_uid(char cid[MAX_CID_SIZE], char uid[MAX_UID_SIZE])
{
	char cmd[32 + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "get CID_UID:%s", cid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_answer_value_t *value = answer_head_value(ans);

	if (answer_value_look_type(value) != VALUE_TYPE_STAR) {
		x_printf(E, "[FAIL] %s", cmd);
		kv_handler_release(handler);
		return -1;
	}

	char    *str = (char *)answer_value_look_addr(value);
	size_t  len = answer_value_look_size(value);

	size_t size = MIN(MAX_UID_SIZE, len + 1);
	snprintf(uid, size, "%s", str);
	kv_handler_release(handler);
	return 0;
}

int exc_cidmap_del_uid(char cid[MAX_CID_SIZE])
{
	char cmd[32 + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "del CID_UID:%s", cid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

/////////////
int exc_cidmap_get_gid(char cid[MAX_CID_SIZE], CID_FOR_EACH_GID fcb, void *usr)
{
	char cmd[32 + MAX_CID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "smembers CID_GID:%s", cid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
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

			char gid[MAX_GID_SIZE] = { 0 };
			memset(gid, 0, sizeof(gid));

			if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
				memcpy(gid, answer_value_look_addr(value), MIN(answer_value_look_size(value), sizeof(gid)));
			} else {
				snprintf(gid, sizeof(gid), "%d", answer_value_look_int(value));
			}

			fcb(gid, idx, usr);
			idx++;
		}

		answer_iter_free(iter);
	}

	kv_handler_release(handler);
	return 0;
}

// FIXME:to fill not fd by of cid
int exc_cidmap_add_gid(char cid[MAX_CID_SIZE], char gid[MAX_GID_SIZE])
{
	char cmd[32 + MAX_CID_SIZE + MAX_GID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "sadd CID_GID:%s %s", cid, gid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

int exc_cidmap_rem_gid(char cid[MAX_CID_SIZE], char gid[MAX_GID_SIZE])
{
	char cmd[32 + MAX_CID_SIZE + MAX_GID_SIZE] = {};

	snprintf(cmd, sizeof(cmd), "srem CID_GID:%s %s", cid, gid);

	kv_handler_t    *handler = kv_spl(g_cid_map, cmd, strlen(cmd));
	kv_answer_t     *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	kv_handler_release(handler);
	return 0;
}

