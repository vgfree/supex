#include <string.h>
#include "keyval.h"
#include "base/utils.h"

#define HKEY_SIZE 100

void keyval_init(void)
{
	kv_init();
	int idx = kv_load(NULL, NULL);
	assert(idx == 0);
}
void keyval_destroy(void)
{
	kv_destroy();
}

static void _int2string(int value, char *buf)
{
	snprintf(buf, 10, "%d", value);
}

/************************************************************/

void key_set_val(char *key, char *value, int size)
{
	if (value == NULL) {
		return;
	}
	char s_key[HKEY_SIZE] = {0};
	snprintf(s_key, sizeof(s_key), "%s:DATA", key);
	kv_handler_t *handler = kv_opt(0, "set", s_key, strlen(s_key), value, size);
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}

char *key_get_val(char *key, int *size)
{
	char s_key[HKEY_SIZE] = {0};
	snprintf(s_key, sizeof(s_key), "%s:DATA", key);
	kv_handler_t *handler = kv_opt(0, "get", s_key, strlen(s_key));
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		*size = 0;
		return NULL;
	}
	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) != VALUE_TYPE_STAR) {
		x_printf(D, "get %s nil value", key);
		kv_handler_release(handler);
		*size = 0;
		return NULL;
	}
	char *str = (char *)answer_value_look_addr(value);
	size_t len = answer_value_look_size(value);

	char *_value = (char *)malloc(len);
	memcpy(_value, str, len);
	*size = len;
	kv_handler_release(handler);
	return _value;
}

void key_del_val(char *key)
{
	char s_key[HKEY_SIZE] = {0};
	snprintf(s_key, sizeof(s_key), "%s:DATA", key);
	kv_handler_t *handler = kv_opt(0, "del", s_key, strlen(s_key));
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}

/************************************************************/
void sfd_set_key(int sfd, char *key)
{
	char buf[10] = {};
	_int2string(sfd, buf);
	char cmd[HKEY_SIZE] = "set ";
	strcat(cmd, buf);
	strcat(cmd, ":MARK ");
	strcat(cmd, key);
	kv_handler_t *handler = kv_spl(0, cmd, strlen(cmd) + 1);
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		error("insert sfd--key error");
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}


char *sfd_get_key(int sfd)
{
	char cmd[HKEY_SIZE] = "get ";
	char buf[10] = {};
	_int2string(sfd, buf);
	strcat(cmd, buf);
	strcat(cmd, ":MARK");
	kv_handler_t *handler = kv_spl(0, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		error("find key error.");
		kv_handler_release(handler);
		return NULL;
	}

	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) != VALUE_TYPE_STAR) {
		x_printf(D, "get %d nil value", sfd);
		kv_handler_release(handler);
		return NULL;
	}
	char *str = (char *)answer_value_look_addr(value);
	size_t len = answer_value_look_size(value);

	char *_value = (char *)malloc(len);
	memcpy(_value, str, len);
	kv_handler_release(handler);
	return _value;
}

void sfd_del_key(int sfd)
{
	char cmd[HKEY_SIZE] = "del ";
	char buf[10] = {};
	_int2string(sfd, buf);
	strcat(cmd, buf);
	strcat(cmd, ":MARK");
	kv_handler_t *handler = kv_spl(0, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		error("delete sfd key failed");
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}

