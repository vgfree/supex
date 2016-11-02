#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gid_map.h"
#include "libmini.h"

static int g_gid_map = 0;

void init_gid_map()
{
	g_gid_map = kv_load(NULL, NULL);
}

int find_fd_list(char *gid, int fd_list[], int *size)
{
	char cmd[30 + MAX_GID_SIZE] = {};
	snprintf(cmd, 30 + MAX_GID_SIZE, "lrange %s 0 -1", gid);

	kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "find multi fd error, cmd:%s", cmd);
		*size = 0;
		kv_answer_release(ans);
		return -1;
	}

	int                     idx = 0;
	kv_answer_value_t       *value;
	kv_answer_iter_t        *iter;

	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	kv_answer_rewind_iter(ans, iter);

	while (((value = kv_answer_next(iter)) != NULL) && (idx < *size)) {
		char buf[20] = {};
		strncpy(buf, (char *)value->ptr, value->ptrlen);
		fd_list[idx] = atoi(buf);
		idx++;
	}

	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	*size = idx;
	return 0;
}

int insert_fd_list(char *gid, int fd_list[], int size)
{
	assert(size <= MAX_ONE_GID_HAVE_CID);
	char cmd[MAX_ONE_GID_HAVE_CID * 10 + 20] = "lpush ";//FIXME:to fill not fd by of cid

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
	char cmd[50 + MAX_GID_SIZE] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 50 + MAX_GID_SIZE);
		snprintf(cmd, 50 + MAX_GID_SIZE, "lrem %s 0 %d", gid, fd_list[i]);
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
	char    cmd[20 + MAX_GID_SIZE] = {};
	snprintf(cmd, 20 + MAX_GID_SIZE, "lpush %d %s", fd, gid);

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
	return 0;
}

int remove_gid_list(int fd, char *gid[], int size)
{
	char cmd[60 + MAX_GID_SIZE] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 60 + MAX_GID_SIZE);
		snprintf(cmd, 60 + MAX_GID_SIZE, "lrem %d 0 %s", fd, gid[i]);
		
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

