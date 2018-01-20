#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libkv.h"
#include "libmini.h"
#include "exc_comm_def.h"
#include "exc_uid_map.h"

static int g_uid_map = 0;

void exc_uidmap_init(void)
{
	g_uid_map = kv_load(NULL, NULL);
}

void exc_uidmap_free(void)
{
}

int exc_uidmap_get_cid(char uid[MAX_UID_SIZE], char cid[MAX_CID_SIZE])
{
	char cmd[32 + MAX_UID_SIZE] = {};
	snprintf(cmd, sizeof(cmd), "get UID_CID:%s", uid);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
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

	char *str = (char *)answer_value_look_addr(value);
	size_t len = answer_value_look_size(value);

	snprintf(cid, MIN(sizeof(cid), len), "%s", str);
	kv_handler_release(handler);

	return 0;
}

int exc_uidmap_set_cid(char uid[MAX_UID_SIZE], char cid[MAX_CID_SIZE])
{
	char cmd[32 + MAX_UID_SIZE + MAX_CID_SIZE] = {};
	snprintf(cmd, sizeof(cmd), "set UID_CID:%s %s", uid, cid);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);
	return 0;
}

int exc_uidmap_del_cid(char uid[MAX_UID_SIZE])
{
	char cmd[32 + MAX_UID_SIZE] = {};
	snprintf(cmd, sizeof(cmd), "del UID_CID:%s", uid);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "[FAIL] %s errnum:%d\terr:%s", cmd, ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);
	return 0;
}
