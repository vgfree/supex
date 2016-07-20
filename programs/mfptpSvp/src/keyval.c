#include <string.h>
#include "keyval.h"
#include "base/utils.h"

#define HKEY_SIZE 100

static kv_handler_t *g_kv_handle = NULL;
void keyval_init(void)
{
	g_kv_handle = kv_create(NULL);
}
void keyval_destroy(void)
{
	kv_destroy(g_kv_handle);
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
	char cmd[HKEY_SIZE] = "set ";
	strcat(cmd, key);
	strcat(cmd, ":DATA ");
	strcat(cmd, value);
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert value error");
	}
	kv_answer_release(ans);
}

char *key_get_val(char *key, int *size)
{
	char cmd[HKEY_SIZE] = "get ";
	strcat(cmd, key);
	strcat(cmd, ":DATA");
	// 不支持二进制.
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("get key:%s error.", key);
		kv_answer_release(ans);
		*size = 0;
		return NULL;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	char *_value = (char *)malloc(value->ptrlen);
	memcpy(_value, value->ptr, value->ptrlen);
	*size = value->ptrlen;
	kv_answer_release(ans);
	return _value;
}

void key_del_val(char *key)
{
	char cmd[HKEY_SIZE] = "del ";
	strcat(cmd, key);
	strcat(cmd, ":DATA");
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("delete value failed");
	}
	kv_answer_release(ans);
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
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert sfd--key error");
	}
	kv_answer_release(ans);
}


char *sfd_get_key(int sfd)
{
	char cmd[HKEY_SIZE] = "get ";
	char buf[10] = {};
	_int2string(sfd, buf);
	strcat(cmd, buf);
	strcat(cmd, ":MARK");
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("find key error.");
		kv_answer_release(ans);
		return NULL;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	x_printf(D, "value->ptrlen:%d", value->ptrlen);
	char *_value = (char *)malloc(value->ptrlen + 1);
	int i = 0;
	for (; i < value->ptrlen; i++) {
		_value[i] = ((char *) value->ptr)[i];
	}
	_value[i] = '\0';
	kv_answer_release(ans);
	return _value;
}

void sfd_del_key(int sfd)
{
	char cmd[HKEY_SIZE] = "del ";
	char buf[10] = {};
	_int2string(sfd, buf);
	strcat(cmd, buf);
	strcat(cmd, ":MARK");
	kv_answer_t *ans = kv_ask(g_kv_handle, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("delete sfd key failed");
	}
	kv_answer_release(ans);
}

