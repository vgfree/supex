#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <unistd.h>
#include "log.h"
#include "imei.h"

#include "grp_pthread.h"
#include "worker_mem.h"

#define         USER_CFG "user.cfg"

typedef struct _USER_TYPE
{
	char    user[16];
	char    thread_idx;		/*之前为协议类型。现在是809协议线程的编号*/
} USER_TYPE;

static USER_TYPE        *g_user_tab;
static int              g_user_cnt;

static int
user_compare(const void *big, const void *small)
{
	USER_TYPE       *b = (USER_TYPE *)big;
	USER_TYPE       *s = (USER_TYPE *)small;

	return strcmp(b->user, s->user);
}

static int
user_init(void)
{
	struct json_object      *obj = NULL;
	struct json_object      *file = NULL;
	struct json_object      *ele = NULL;
	int                     len = 0;
	int                     i = 0;
	char                    cfg_file[256] = {};
	int                     thread_idx = -1;

	snprintf(cfg_file, sizeof(cfg_file), "%s/etc/%s", getenv("WORK_PATH"), USER_CFG);

	if (0 != access(cfg_file, F_OK)) {
		log_info(LOG_E, "文件[%s]不存在\n", cfg_file);
		return -1;
	}

	file = json_object_from_file(cfg_file);

	if (is_error(file)) {
		log_info(LOG_E, "打开配置文件[%s]失败\n", cfg_file);
		return -1;
	}

	obj = json_object_object_get(file, "user_thread_idx");

	if (is_error(obj)) {
		log_info(LOG_E, "获取指定属性user_thread_idx失败\n");
		json_object_put(file);
		return -1;
	}

	len = json_object_array_length(obj);
	g_user_cnt = len;
	g_user_tab = (USER_TYPE *)calloc(g_user_cnt, sizeof(USER_TYPE));

	if (NULL == g_user_tab) {
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
			if (strlen(key) > 16 - 1) {
				log_info(LOG_E, "user[%s]长度超长\n", key);
				json_object_put(ele);
				json_object_put(file);
				json_object_put(obj);
				return -1;
			}

			strcpy(g_user_tab[i].user, key);
			thread_idx = json_object_get_int(val);
			g_user_tab[i].thread_idx = (char)thread_idx;
		}
		json_object_put(ele);
	}

	/*排序*/
	qsort(g_user_tab, g_user_cnt, sizeof(USER_TYPE), user_compare);
	json_object_put(file);
	json_object_put(obj);

	return 0;
}

/*
 *功能：读取配置文件初始化内存
 */
int
mem_init(void)
{
	if (-1 == imei_init()) {
		log_info(LOG_E, "error:imei_init\n");
		return -1;
	}

	if (-1 == user_init()) {
		log_info(LOG_E, "error:user_init\n");
		return -1;
	}

	return 0;
}

/*
 *功能：释放内存
 * */
int mem_term(void)
{
	imei_term();
	g_user_tab && (free(g_user_tab), 1);

	return 0;
}

static int
thread_search(char *user, char *thread_idx)
{
	USER_TYPE       key = {};
	USER_TYPE       *val = NULL;

	strcpy(key.user, user);
	val = bsearch(&key, g_user_tab, g_user_cnt, sizeof(USER_TYPE), user_compare);

	if (NULL == val) {
		return -1;
	}

	*thread_idx = val->thread_idx;
	return 0;
}

/*
 *功能：根据imei获取该imei属于哪个用户，该用户采用什么协议
 * 入参：
 *	imei
 *出参：
 *	type
 *	user
 * 返回值：
 *      0
 *	-1
 * */
int
mem_thread_get(char *imei, char *thread_idx, char *user)
{
	assert(imei && thread_idx && user);

	if (-1 == user_search(imei, user)) {
		return -1;
	}

	if (-1 == thread_search(user, thread_idx)) {
		return -1;
	}

	return 0;
}

