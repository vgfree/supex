#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "log.h"
#include <json.h>
#include "imei.h"

#define         IMEI_CFG "imei.cfg"

int (*worker_timer_do)(void);

static int
imei_compare(const void *big, const void *small)
{
	IMEI_USER       *b = (IMEI_USER *)big;
	IMEI_USER       *s = (IMEI_USER *)small;

	return strcmp(b->imei, s->imei);
}

IMEI_USER       *g_imei_tab;
int             g_imei_cnt;

int imei_init(void)
{
	struct json_object      *obj = NULL;
	struct json_object      *file = NULL;
	struct json_object      *ele = NULL;
	int                     len = 0;
	int                     i = 0;
	char                    cfg_file[256] = {};
	const char              *user;

	worker_timer_do = NULL;

	snprintf(cfg_file, sizeof(cfg_file), "%s/etc/%s", getenv("WORK_PATH"), IMEI_CFG);

	if (0 != access(cfg_file, F_OK)) {
		log_info(LOG_E, "文件[%s]不存在\n", cfg_file);
		return -1;
	}

	file = json_object_from_file(cfg_file);

	if (is_error(file)) {
		return -1;
	}

	obj = json_object_object_get(file, "imei_user");

	if (is_error(obj)) {
		json_object_put(file);
		log_info(LOG_E, "[%s]中找不到[imei_user]\n", cfg_file);
		return -1;
	}

	len = json_object_array_length(obj);
	g_imei_cnt = len;
	g_imei_tab = (IMEI_USER *)calloc(g_imei_cnt, sizeof(IMEI_USER));

	if (NULL == g_imei_tab) {
		log_info(LOG_E, "内存分配错\n");
		return -1;
	}

	/*遍历数组*/
	for (i = 0; i < len; i++) {
		ele = json_object_array_get_idx(obj, i);

		if (is_error(ele)) {
			json_object_put(file);
			json_object_put(obj);
			return -1;
		}

		json_object_object_foreach(ele, key, val)
		{
			/*装入内存*/
			user = json_object_get_string(val);

			if ((strlen(key) > 16 - 1) || (strlen(user) > 16 - 1)) {
				printf("imei[%s]或user[%s]长度超长\n", key, user);
				json_object_put(ele);
				json_object_put(obj);
				json_object_put(file);
				return -1;
			}

			strcpy(g_imei_tab[i].imei, key);
			strcpy(g_imei_tab[i].user, user);
		}
		json_object_put(ele);
	}

	/*排序*/
	qsort(g_imei_tab, g_imei_cnt, sizeof(IMEI_USER), imei_compare);
	json_object_put(file);
	json_object_put(obj);

	return 0;
}

void imei_term(void)
{
	g_imei_tab && (free(g_imei_tab), 1);
}

int user_search(char *imei, char *user)
{
	IMEI_USER       key = {};
	IMEI_USER       *val = NULL;

	strcpy(key.imei, imei);
	val = bsearch(&key, g_imei_tab, g_imei_cnt, sizeof(IMEI_USER), imei_compare);

	if (NULL == val) {
		return -1;
	}

	strcpy(user, val->user);
	return 0;
}

