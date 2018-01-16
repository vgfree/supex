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

	kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(E, "find multi fd error, cmd:%s", cmd);
		*size = 0;
		kv_handler_release(handler);
		return -1;
        }

	int                     idx = 0;
	unsigned long           len = answer_length(ans);
	if (len != 1) {
		kv_answer_iter_t        *iter = answer_iter_make(ans, ANSWER_HEAD);

		kv_answer_value_t       *value;
		while (((value = answer_iter_next(iter)) != NULL) && (idx < *size)) {
			if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
				x_printf(D, "this IMEI has not data!\n");
				answer_iter_free(iter);
				abort();
			}

			if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
				char ptr[128] = {0};
				memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
				fd_list[idx] = atoi(ptr);
			} else {
				fd_list[idx] = answer_value_look_int(value);
			}
			idx++;
		}


		answer_iter_free(iter);
	}

	kv_handler_release(handler);
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

	kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }

	kv_handler_release(handler);

	return 0;
}

int remove_fd_list(char *gid, int fd_list[], int size)
{
	char cmd[50 + MAX_GID_SIZE] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 50 + MAX_GID_SIZE);
		snprintf(cmd, 50 + MAX_GID_SIZE, "lrem %s 0 %d", gid, fd_list[i]);
		x_printf(D, "%s", cmd);

		kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
		kv_answer_t *ans = &handler->answer;

		if (ERR_NONE != ans->errnum) {
			x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
			kv_handler_release(handler);
			return -1;
		}

		kv_handler_release(handler);
	}

	return 0;
}

int insert_gid_list(int fd, char *gid)
{
	char    cmd[20 + MAX_GID_SIZE] = {};
	snprintf(cmd, 20 + MAX_GID_SIZE, "lpush %d %s", fd, gid);

	kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }

	kv_handler_release(handler);
	return 0;
}

int find_gid_list(int fd, char *gid_list[], int *size)
{
	char    cmd[40] = {};
	snprintf(cmd, 40, "lrange %d 0 -1", fd);
	
	kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		x_printf(I, "find multi gid error, cmd:%s\n", cmd);
		*size = 0;
		kv_handler_release(handler);
		return -1;
        }

	int                     idx = 0;
	unsigned long           len = answer_length(ans);
	if (len != 1) {
		kv_answer_iter_t        *iter = answer_iter_make(ans, ANSWER_HEAD);

		kv_answer_value_t       *value;
		while (((value = answer_iter_next(iter)) != NULL) && (idx < *size)) {
			if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
				x_printf(D, "this IMEI has not data!\n");
				answer_iter_free(iter);
				abort();
			}

			char *addr = answer_value_look_addr(value);
			size_t size = answer_value_look_size(value);
			gid_list[idx] = (char *)malloc(size * sizeof(char) + 1);
			strncpy(gid_list[idx], (char *)addr, size);
			gid_list[idx][size] = '\0';
			idx++;
		}


		answer_iter_free(iter);
	}

	kv_handler_release(handler);
	*size = idx;
	return 0;
}

int remove_gid_list(int fd, char *gid[], int size)
{
	char cmd[60 + MAX_GID_SIZE] = {};
	for (int i = 0; i < size; i++) {
		memset(cmd, 0, 60 + MAX_GID_SIZE);
		snprintf(cmd, 60 + MAX_GID_SIZE, "lrem %d 0 %s", fd, gid[i]);
		
		kv_handler_t *handler = kv_spl(g_gid_map, cmd, strlen(cmd));
		kv_answer_t *ans = &handler->answer;
	
		if (ERR_NONE != ans->errnum) {
			x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
			kv_handler_release(handler);
			return -1;
		}

		kv_handler_release(handler);
	}

	return 0;
}

void destroy_gid_map()
{
}

