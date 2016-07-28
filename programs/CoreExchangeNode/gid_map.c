#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gid_map.h"
#include "libmini.h"

static kv_handler_t *g_gid_map = NULL;

void init_gid_map()
{
	g_gid_map = kv_create(NULL);
}

int find_fd_list(char *gid, int *fd_list)//TODO size
{
	char cmd[30] = "lrange ";

	strcat(cmd, gid);
	strcat(cmd, " 0 -1");
	kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "find multi fd error, cmd:%s", cmd);
		kv_answer_release(ans);
		return -1;
	}

	int                     i = 0;
	kv_answer_value_t       *value;
	kv_answer_iter_t        *iter;

	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	kv_answer_rewind_iter(ans, iter);

	while ((value = kv_answer_next(iter)) != NULL) {
		char buf[20] = {};
		strncpy(buf, (char *)value->ptr, value->ptrlen);
		fd_list[i] = atoi(buf);
		i++;
	}

	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	return i;
}

int insert_fd_list(char *gid, int fd_list[], int size)
{
	char cmd[GROUP_SIZE * 10 + 20] = "lpush ";

	strcat(cmd, gid);

	for (int i = 0; i < size; i++) {
		strcat(cmd, " ");
		char buf[10];
		snprintf(buf, 10, "%d", fd_list[i]);
		strcat(cmd, buf);
	}

	kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);

	return 0;
}

int remove_fd_list(char *gid, int fd_list[], int size)
{
	char cmd[50] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 50);
		snprintf(cmd, 50, "lrem %s 0 %d", gid, fd_list[i]);
		x_printf(D, "%s", cmd);

		kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
		if (ans->errnum != ERR_NONE) {
			x_printf(E, "cmd:%s, errnum:%d\terr:%s\n",
				cmd, ans->errnum, ans->err);
			kv_answer_release(ans);
			return -1;
		}

		kv_answer_release(ans);
	}

	return 0;
}

int insert_gid_list(int fd, char *gid)
{
	char    cmd[20 + 20] = "lpush ";
	char    buf[10] = {};

	snprintf(buf, 10, "%d", fd);
	strcat(cmd, buf);
	strcat(cmd, " ");
	strcat(cmd, gid);
	kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);
	return 0;
}

int find_gid_list(int fd, char *gid_list[], int *size)
{
	char    cmd[40] = {};
	snprintf(cmd, 40, "lrange %d 0 -1", fd);
	
	kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
	if (ans->errnum != ERR_NONE) {
		x_printf(I, "find multi gid error, cmd:%s\n", cmd);
		*size = 0;
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_iter_t        *iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	kv_answer_rewind_iter(ans, iter);

	int                     idx = 0;
	kv_answer_value_t       *value = NULL;
	while (((value = kv_answer_next(iter)) != NULL) && (idx < *size)) {
		gid_list[idx] = (char *)malloc(value->ptrlen * sizeof(char) + 1);
		strncpy(gid_list[idx], (char *)value->ptr, value->ptrlen);
		gid_list[idx][value->ptrlen] = '\0';
		idx++;
	}

	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	*size = idx;
	return idx;
}

int remove_gid_list(int fd, char *gid[], int size)
{
	char cmd[60] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 60);
		snprintf(cmd, 60, "lrem %d 0 %s", fd, gid[i]);
		
		kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
		if (ans->errnum != ERR_NONE) {
			x_printf(E, "errnum:%d\terr:%s\n", ans->errnum, ans->err);
			kv_answer_release(ans);
			return -1;
		}

		kv_answer_release(ans);
	}

	return 0;
}

void destroy_gid_map()
{
	kv_destroy(g_gid_map);
}

