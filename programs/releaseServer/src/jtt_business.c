/*
 *   模块说明：
 *        809协议线程处理模块
 */
#include <stdio.h>
#include <sys/time.h>
#include <cJSON.h>
#include <json.h>

#include "jtt_business.h"
#include "jtt_json_to_gps.h"
#include "jtt_client.h"
#include "jtt_package.h"
#include "jtt_body.h"
#include "jtt_comm.h"
#include "log.h"
#include "async_comm.h"

#define JTT_BUSINESS_CFG        "jtt_business.cfg"
#define IN_GPS_NUM              5		/*输入的每个数据包中实时GPS点的数量，由于是每5秒上传一次所以值固定*/

typedef struct
{
	int             thread_idx;		/*线程编号*/
	PKG_USER        pkg_user_info;
	BODY_USER       body_user_info;
} USER_INFO;

USER_INFO       *g_user_info_tab;
int             g_user_info_cnt;

static int user_compare(const void *big, const void *small)
{
	USER_INFO       *b = (USER_INFO *)big;
	USER_INFO       *s = (USER_INFO *)small;

	return strcmp(b->body_user_info.srv_id, s->body_user_info.srv_id);
}

#if 0	/*改为每5个点采集一个点*/

/*
 *   发送频率控制
 *        space:发送间隔时间
 *        last:上传发送数据的时间
 */
int is_allow_send(const struct timeval space, struct timeval *last)
{
	struct timeval now;

	if (-1 == gettimeofday(&now, NULL)) {
		/*出错允许发送*/
		return 1;
	}

	if (now.tv_sec - last->tv_sec > space.tv_sec) {
		*last = now;
		return 1;
	} else if (now.tv_sec - last->tv_sec < space.tv_sec) {
		return 0;
	} else {
		if (now.tv_usec - last->tv_usec > space.tv_usec) {
			*last = now;
			return 1;
		} else if (now.tv_usec - last->tv_usec < space.tv_usec) {
			return 0;
		} else {
			*last = now;
			return 1;
		}
	}

	return 1;
}
#endif	/* if 0 */

/*心跳处理*/
int jtt809_do_heart(time_t space, int idx)
{
	int     i = 0;
	time_t  cur;

	time(&cur);

	for (i = 0; i < g_user_info_cnt; i++) {
		if (idx == g_user_info_tab[i].thread_idx) {
			if (async_user_is_online(g_user_info_tab[i].body_user_info)) {
				if (-1 == jtt_heart(&g_user_info_tab[i].pkg_user_info, &g_user_info_tab[i].body_user_info)) {
					async_user_reset(&g_user_info_tab[i].body_user_info);
					log_info(LOG_F, "用户[%s]心跳发送失败\n", g_user_info_tab[i].body_user_info.srv_id);
					continue;
				}

				log_info(LOG_I, "heart to [%s]\n", g_user_info_tab[i].body_user_info.srv_id);
			}
		}
	}

	return 0;
}

/*初始化配置文件中的用户信息装入内存,并连接上位机*/
int jtt809_out_once_init(void)
{
	struct json_object      *file = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *one = NULL;

	char    cfg_file[256] = {};
	int     len = 0;
	int     i, j;

	snprintf(cfg_file, sizeof(cfg_file), "%s/etc/%s", getenv("WORK_PATH"), JTT_BUSINESS_CFG);

	if (0 != access(cfg_file, F_OK)) {
		log_info(LOG_E, "文件[%s]不存在\n", cfg_file);
		return -1;
	}

	file = json_object_from_file(cfg_file);

	if (is_error(file)) {
		log_info(LOG_E, "打开配置文件错误[%s]\n", cfg_file);
		return -1;
	}

	obj = json_object_object_get(file, "jtt_business_user");

	if (is_error(obj)) {
		log_info(LOG_E, "获取对象jtt_business_user失败\n");
		return -1;
	}

	len = json_object_array_length(obj);
	g_user_info_cnt = len;
	g_user_info_tab = (USER_INFO *)calloc(g_user_info_cnt, sizeof(USER_INFO));

	if (NULL == g_user_info_tab) {
		log_info(LOG_E, "内存分配错误\n");
		return -1;
	}

	for (i = 0; i < len; i++) {
		one = json_object_array_get_idx(obj, i);

		if (is_error(one)) {
			log_info(LOG_E, "取指定索引的对象失败\n");
			return -1;
		}

		j = 0;
		json_object_object_foreach(one, key, val)
		{
			if (!strcmp(key, "encrypt")) {
				g_user_info_tab[i].pkg_user_info.encrypt = json_object_get_int(val);
			} else if (!strcmp(key, "thread_idx")) {
				g_user_info_tab[i].thread_idx = json_object_get_int(val);
			} else if (!strcmp(key, "msg_enterid")) {
				g_user_info_tab[i].pkg_user_info.msg_enterid = (unsigned)json_object_get_int(val);
			} else if (!strcmp(key, "srv_id")) {
				strcpy(g_user_info_tab[i].body_user_info.srv_id, json_object_get_string(val));
			} else if (!strcmp(key, "ip")) {
				strcpy(g_user_info_tab[i].body_user_info.ip, json_object_get_string(val));
			} else if (!strcmp(key, "port")) {
				g_user_info_tab[i].body_user_info.port = (short)json_object_get_int(val);
			} else if (!strcmp(key, "uid")) {
				g_user_info_tab[i].body_user_info.uid = (unsigned)json_object_get_int(val);
			} else if (!strcmp(key, "passwd")) {
				strcpy(g_user_info_tab[i].body_user_info.passwd, json_object_get_string(val));
			} else if (!strcmp(key, "sd_space_sec")) {
				g_user_info_tab[i].body_user_info.sd_space.tv_sec = (unsigned)json_object_get_int(val);
			} else if (!strcmp(key, "sd_space_usec")) {
				g_user_info_tab[i].body_user_info.sd_space.tv_usec = (unsigned)json_object_get_int(val);
			}

			j++;
		}

		g_user_info_tab[i].body_user_info.heart_cnt = 0;
		async_user_init(&g_user_info_tab[i].body_user_info);
		g_user_info_tab[i].body_user_info.sockfd = -1;
		g_user_info_tab[i].body_user_info.last_space.tv_sec = 0;
		g_user_info_tab[i].body_user_info.last_space.tv_usec = 0;

		g_user_info_tab[i].pkg_user_info.head_flag = '[';
		g_user_info_tab[i].pkg_user_info.end_flag = ']';
		VERSION(g_user_info_tab[i].pkg_user_info.version);

		json_object_put(one);
	}

	json_object_put(obj);
	json_object_put(file);

	qsort(g_user_info_tab, g_user_info_cnt, sizeof(USER_INFO), user_compare);

#if 0
	/*测试---------bgn*/
	for (i = 0; i < len; i++) {
		log_info(LOG_I, "encrypt[%d],msg_enterid[%d],"
			"srv_id[%s],ip[%s],port[%d],uid[%d],passwd[%s]\n",
			g_user_info_tab[i].pkg_user_info.encrypt,
			g_user_info_tab[i].pkg_user_info.msg_enterid,
			g_user_info_tab[i].body_user_info.srv_id,
			g_user_info_tab[i].body_user_info.ip,
			g_user_info_tab[i].body_user_info.port,
			g_user_info_tab[i].body_user_info.uid,
			g_user_info_tab[i].body_user_info.passwd
			);
	}
	/*测试---------end*/
#endif

	return 0;
}

USER_INFO *user_info_search(char *userid)
{
	USER_INFO user;

	strcpy(user.body_user_info.srv_id, userid);

	return (USER_INFO *)bsearch(&user, g_user_info_tab, g_user_info_cnt, sizeof(USER_INFO), user_compare);
}

int jtt809_data_handle(const void *data)
{
	NODE            *node;
	USER_INFO       *user;
	BODY_GPS        gps[IN_GPS_NUM] = {};
	BODY_GPS        *ex = NULL;
	BODY_GPS        **extra = &ex;
	int             n_gps = IN_GPS_NUM;
	int             n_extra = 0;
	int             ret;

	node = (NODE *)data;
	log_info(LOG_I, "809协议线程收到用户[%s]的数据\n", node->userid);
	/*查找用户信息*/
	user = user_info_search(node->userid);

	if (NULL == user) {
		log_info(LOG_E, "没有找到用户[%s]的配置信息\n", node->userid);
		cJSON_Delete(node->data), node->data = NULL;
		return -1;
	}

	/*受控连接用户*/
	ret = async_user_connect(&user->pkg_user_info, &user->body_user_info);

	if (-1 == ret) {
		log_info(LOG_E, "连接用户错\n");
		cJSON_Delete(node->data), node->data = NULL;
		return -1;
	} else if (1 == ret) {
		log_info(LOG_D, "等待下次连接\n");
		cJSON_Delete(node->data), node->data = NULL;
		return 0;
	}

#if 0
	/*发送频率控制*/
	if (!is_allow_send(user->body_user_info.sd_space, &user->body_user_info.last_space)) {
		log_info(LOG_D, "频率控制不发送\n");
		cJSON_Delete(node->data), node->data = NULL;
		return 0;
	}
	log_info(LOG_D, "要发送--------->\n");
#endif

	/*解析数据*/
	if (-1 == json_to_gps_data((cJSON *)node->data, (BODY_GPS *)gps, &n_gps, extra, &n_extra)) {
		log_info(LOG_E, "JSON数据转换为809GPS数据失败\n");
		cJSON_Delete(node->data), node->data = NULL;
		return -1;
	}

	cJSON_Delete(node->data), node->data = NULL;

	/*实时数据*/
	if (-1 == jtt_real_send(&user->pkg_user_info, user->body_user_info, gps, n_gps)) {
		*extra && (free(*extra), 1);
		async_user_reset(&user->body_user_info);
		log_info(LOG_E, "向[%s]发送实时数据失败\n", user->body_user_info.srv_id);
		return -1;
	}

	/*补传数据*/
	if (NULL == *extra) {
		return 0;
	}

	if (-1 == jtt_extra_send(&user->pkg_user_info, user->body_user_info, *extra, n_extra)) {
		free(*extra);
		async_user_reset(&user->body_user_info);
		log_info(LOG_E, "向[%s]发送补传数据失败\n", user->body_user_info.srv_id);
		return -1;
	}

	free(*extra);
	return 0;
}

