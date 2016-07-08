#include "hkey.h"
#include "loger.h"
#include "string.h"

#define HKEY_SIZE 100

static kv_handler_t *g_hkey = NULL;
void hkey_init()
{
	g_hkey = kv_create(NULL);
}

static void _int2string(int value, char *buf)
{
	snprintf(buf, 10, "%d", value);
}

void hkey_insert_fd(char *key, int fd)
{
	char buf[10] = {};
	_int2string(fd, buf);
	char cmd[HKEY_SIZE] = "hset ";
	strcat(cmd, key);
	strcat(cmd, " fd ");
	strcat(cmd, buf);
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert fd error");
	}
	kv_answer_release(ans);

	memset(cmd, 0, HKEY_SIZE);
	memcpy(cmd, "hset fd ", 8);	
	strcat(cmd, buf);
	strcat(cmd, " ");
	strcat(cmd, key);
	ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert fd--key error");
	}
	kv_answer_release(ans);
}

int hkey_get_fd(char *key)
{
	char cmd[HKEY_SIZE] = "hget ";
	strcat(cmd, key);
	strcat(cmd, " fd");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		warn("get key:%s error.", key);
		kv_answer_release(ans);
		return -1;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	int fd = atoi((char *)value->ptr);
	kv_answer_release(ans);
	return fd;
}

void hkey_del_fd(char *key)
{
	char cmd[HKEY_SIZE] = "hdel ";
	strcat(cmd, key);
	strcat(cmd, " fd");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		error("delete fd failed");
	}
	kv_answer_release(ans);
}

void hkey_insert_value(char *key, char *value)
{
	char cmd[HKEY_SIZE] = "hset ";
	strcat(cmd, key);
	strcat(cmd, " value ");
	strcat(cmd,	value);
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert value error");
	}
	kv_answer_release(ans);
}

char *hkey_get_value(char *key)
{
	char cmd[HKEY_SIZE] = "hget ";
	strcat(cmd, key);
	strcat(cmd, " value");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("get key:%s error.", key);
		kv_answer_release(ans);
		return -1;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	char *_value = (char *)malloc(value->ptrlen + 1);
	int i = 0;
	for (; i < value->ptrlen; i++) {
		_value[i] = ((char *) value->ptr)[i];
	}
	_value[i] = '\0';
	kv_answer_release(ans);
	return _value;
}

void hkey_del_value(char *key)
{
	char cmd[HKEY_SIZE] = "hdel ";
	strcat(cmd, key);
	strcat(cmd, " value");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("delete value failed");
	}
	kv_answer_release(ans);
}

void hkey_insert_offset(char *key, int offset)
{
	char buf[10] = {};
	_int2string(offset, buf);
	char cmd[HKEY_SIZE] = "hset ";
	strcat(cmd, key);
	strcat(cmd, " offset ");
	strcat(cmd, buf);
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("insert offset error");
	}
	kv_answer_release(ans);
}

int hkey_get_offset(char *key)
{
	char cmd[HKEY_SIZE] = "hget ";
	strcat(cmd, key);
	strcat(cmd, " offset");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("get key:%s error.", key);
		kv_answer_release(ans);
		return 0;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	int offset = atoi((char *)value->ptr);
	kv_answer_release(ans);
	return offset;
}

void hkey_del_offset(char *key)
{
	char cmd[HKEY_SIZE] = "hdel ";
	strcat(cmd, key);
	strcat(cmd, " offset");
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		error("delete offset failed");
	}
	kv_answer_release(ans);
}

void hkey_destroy()
{
	kv_destroy(g_hkey);
}

char *hkey_find_key(int fd)
{
	char cmd[HKEY_SIZE] = "hget fd ";
	char buf[10] = {};
	_int2string(fd, buf);
	strcat(cmd, buf);
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("find key error.");
		kv_answer_release(ans);
		return -1;
	}
	kv_answer_value_t *value = kv_answer_first_value(ans);
	char *_value = (char *)malloc(value->ptrlen + 1);
	int i = 0;
	for (; i < value->ptrlen; i++) {
		_value[i] = ((char *) value->ptr)[i];
	}
	_value[i] = '\0';
	kv_answer_release(ans);
	return _value;
}

void hkey_del_fd_key(int fd)
{
	char cmd[HKEY_SIZE] = "hdel fd ";
	char buf[10] = {};
	_int2string(fd, buf);
	strcat(cmd, buf);
	kv_answer_t *ans = kv_ask(g_hkey, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		error("delete fd key failed");
	}
	kv_answer_release(ans);

}

