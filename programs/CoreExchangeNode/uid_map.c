#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "uid_map.h"
#include "libmini.h"

static kv_handler_t *g_uid_map = NULL;

void init_uid_map()
{
	g_uid_map = kv_create(NULL);
}

int find_fd(char *uid)
{
	char cmd[20] = "get ";

	strcat(cmd, uid);
	kv_answer_t *ans;
	ans = kv_ask(g_uid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_value_t       *value = kv_answer_first_value(ans);
	int                     fd = atoi((char *)value->ptr);
	kv_answer_release(ans);
	return fd;
}

int find_uid(char *uid, int *size, int fd)
{
	char    cmd[20] = "get ";
	char    buf[10] = {};

	snprintf(buf, 10, "%d", fd);
	strcat(cmd, buf);
	kv_answer_t *ans;
	ans = kv_ask(g_uid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_value_t       *value = kv_answer_first_value(ans);
	size_t                  i = 0;

	for (; i < value->ptrlen; i++) {
		uid[i] = ((char *)value->ptr)[i];
	}

	kv_answer_release(ans);
	*size = i;
	return i;
}

int insert_fd(char *uid, int fd)
{
	char cmd[60] = {};
	snprintf(cmd, 60, "set %s %d", uid, fd);

	kv_answer_t *ans = kv_ask(g_uid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "failed:%s.\n", cmd);
		kv_answer_release(ans);
		return -1;
	}

	char uid_cmd[50] = {};
	snprintf(uid_cmd, 50, "set %d %s", fd, uid);
	kv_answer_t *uid_ans = kv_ask(g_uid_map, uid_cmd, strlen(uid_cmd));

	if (uid_ans->errnum != ERR_NONE) {
		x_printf(E, "insert fd:%d, uid:%s error.\n", fd, uid);
		kv_answer_release(uid_ans);
		return -1;
	}

	kv_answer_release(ans);
	kv_answer_release(uid_ans);

	return 0;
}

int remove_fd(char *uid)
{
	char cmd[20] = "del ";

	strcat(cmd, uid);
	kv_answer_t *ans = kv_ask(g_uid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "removed uid:%s error.\n", uid);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);

	return 0;
}

int remove_uid(int fd)
{
	char    cmd[20] = "del ";
	char    buf[10] = {};

	snprintf(buf, 10, "%d", fd);
	strcat(cmd, buf);
	kv_answer_t *ans = kv_ask(g_uid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "removed fd:%d error.\n", fd);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);
	return 0;
}

void destroy_uid_map()
{
	kv_destroy(g_uid_map);
}

