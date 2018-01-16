#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "uid_map.h"
#include "libmini.h"
#include "comm_def.h"

static int g_uid_map = 0;

void init_uid_map()
{
	g_uid_map = kv_load(NULL, NULL);
}

int find_fd(char *uid)
{
	char cmd[20 + MAX_UID_SIZE] = "get ";
	strcat(cmd, uid);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }

	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) != VALUE_TYPE_STAR) {
		x_printf(D, "get %s nil value", uid);
		kv_handler_release(handler);
		return -1;
	}
	char *str = (char *)answer_value_look_addr(value);
	size_t len = answer_value_look_size(value);

	char *_value = (char *)calloc(len + 1, sizeof(char));
	memcpy(_value, str, len);
	kv_handler_release(handler);

	int fd = atoi((char *)_value);
	free(_value);
	return fd;
}

int find_uid(char *uid, int *size, int fd)
{
	char    cmd[20] = {};
	snprintf(cmd, 20, "get %d", fd);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }

	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) != VALUE_TYPE_STAR) {
		x_printf(D, "get %d nil value", fd);
		kv_handler_release(handler);
		return -1;
	}
	char *str = (char *)answer_value_look_addr(value);
	size_t len = answer_value_look_size(value);

	memcpy(uid, str, len);
	*size = len;
	kv_handler_release(handler);

	return len;
}

int insert_fd(char *uid, int fd)
{
	char cmd[60 + MAX_UID_SIZE] = {};
	snprintf(cmd, 60 + MAX_UID_SIZE, "set %s %d", uid, fd);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(E, "failed:%s.\n", cmd);
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);



	char uid_cmd[50 + MAX_UID_SIZE] = {};
	snprintf(uid_cmd, 50 + MAX_UID_SIZE, "set %d %s", fd, uid);

	handler = kv_spl(g_uid_map, uid_cmd, strlen(uid_cmd));
	ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(E, "insert fd:%d, uid:%s error.\n", fd, uid);
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);

	return 0;
}

int remove_fd(char *uid)
{
	char cmd[20 + MAX_UID_SIZE] = "del ";
	strcat(cmd, uid);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(E, "removed uid:%s error.\n", uid);
                kv_handler_release(handler);
		return -1;
        }

	kv_handler_release(handler);
	return 0;
}

int remove_uid(int fd)
{
	char    cmd[20] = {};
	snprintf(cmd, 20, "del %d", fd);

	kv_handler_t *handler = kv_spl(g_uid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(E, "removed fd:%d error.\n", fd);
                kv_handler_release(handler);
		return -1;
        }

	kv_handler_release(handler);
	return 0;
}

void destroy_uid_map()
{
}

