#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <my_global.h>
#include <mysql.h>
#include "imei.h"
#include <pthread.h>
#include <json.h>
#include "log.h"

#define SQL_LEN         2048
#define MYSQL_CFG       "mysql.cfg"

int (*worker_timer_do)(void);

typedef struct MYSQL_INFO
{
	char    ip[16];
	char    user[32];
	char    passwd[32];
	char    database[128];
	char    table[128];
} MYSQL_INFO;

typedef struct MYSQL_IMEI_USER
{
	int     cnt;
	char    **ptr_tab;
} MYSQL_IMEI_USER;

MYSQL_INFO      g_mysql_info;
MYSQL_IMEI_USER g_mysql_imei_user;		/*第三方公司的businessid*/

IMEI_USER               *g_imei_tab;
int                     g_imei_cnt;
pthread_rwlock_t        rwlock;

static int
imei_compare(const void *big, const void *small)
{
	IMEI_USER       *b = (IMEI_USER *)big;
	IMEI_USER       *s = (IMEI_USER *)small;

	return strcmp(b->imei, s->imei);
}

int mysql_info_init(void)
{
	/*
	 *        strcpy(g_mysql_info.ip,"192.168.1.6");
	 *        strcpy(g_mysql_info.user,"root");
	 *        strcpy(g_mysql_info.passwd,"Agh34GHHJklHDDdEAb");
	 *        strcpy(g_mysql_info.database,"crowdRewards");
	 *        strcpy(g_mysql_info.table,"businessMirrtalkInfo");
	 *        g_mysql_imei_user.cnt = 2;
	 *        g_mysql_imei_user.ptr_tab = (char **)calloc(g_mysql_imei_user.cnt,sizeof(char *));
	 *        if(!g_mysql_imei_user.ptr_tab)
	 *                return -1;
	 *
	 *        g_mysql_imei_user.ptr_tab[0] = "1";
	 *        g_mysql_imei_user.ptr_tab[1] = "2";
	 */
	int                     i;
	const char              *str = NULL;
	char                    cfg_file[256] = {};
	struct json_object      *file = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *one = NULL;

	snprintf(cfg_file, sizeof(cfg_file), "%s/etc/%s", getenv("WORK_PATH"), MYSQL_CFG);
	// snprintf(cfg_file,sizeof(cfg_file),"./%s",MYSQL_CFG);

	if (0 != access(cfg_file, F_OK)) {
		log_info(LOG_E, "配置文件[%s]不存在\n", cfg_file);
		return -1;
	}

	file = json_object_from_file(cfg_file);

	if (is_error(file)) {
		log_info(LOG_E, "json_object_from_file打开配置文件[%s]错误\n", cfg_file);
		return -1;
	}

	json_object_object_foreach(file, key, val)
	{
		if (!strcmp(key, "ip")) {
			strcpy(g_mysql_info.ip, json_object_get_string(val));
		} else if (!strcmp(key, "user")) {
			strcpy(g_mysql_info.user, json_object_get_string(val));
		} else if (!strcmp(key, "passwd")) {
			strcpy(g_mysql_info.passwd, json_object_get_string(val));
		} else if (!strcmp(key, "database")) {
			strcpy(g_mysql_info.database, json_object_get_string(val));
		} else if (!strcmp(key, "table")) {
			strcpy(g_mysql_info.table, json_object_get_string(val));
		} else if (!strcmp(key, "business_id")) {
			obj = json_object_object_get(file, "business_id");

			if (is_error(obj)) {
				log_info(LOG_E, "从配置文件[%s]中获取对象business_id失败\n", cfg_file);
				json_object_put(file);
				return -1;
			}

			g_mysql_imei_user.cnt = json_object_array_length(obj);
			g_mysql_imei_user.ptr_tab = (char **)calloc(g_mysql_imei_user.cnt, sizeof(char *));

			if (!g_mysql_imei_user.ptr_tab) {
				log_info(LOG_E, "内存分配错\n");
				json_object_put(obj);
				json_object_put(file);
				return -1;
			}

			for (i = 0; i < g_mysql_imei_user.cnt; i++) {
				one = json_object_array_get_idx(obj, i);

				if (is_error(one)) {
					log_info(LOG_E, "从配置文件[%s]中获取对象business_id中指定索引的对象错\n", cfg_file);
					json_object_put(obj);
					json_object_put(file);
					return -1;
				}

				str = json_object_get_string(one);
				g_mysql_imei_user.ptr_tab[i] = (char *)calloc(1, strlen(str) + 1);
				strcpy(g_mysql_imei_user.ptr_tab[i], str);

				json_object_put(one);
			}

			json_object_put(obj);
		}
	}

	json_object_put(file);
	return 0;
}

MYSQL *mysql_mem_connect(void)
{
	MYSQL *conn = NULL;

	conn = mysql_init(NULL);

	if (!conn) {
		return NULL;
	}

	if (!mysql_real_connect(conn, g_mysql_info.ip, g_mysql_info.user, g_mysql_info.passwd, g_mysql_info.database, 0, NULL, 0)) {
		log_info(LOG_E, "mysql_real_connect error:%s\n", mysql_error(conn));
		mysql_close(conn);
		return NULL;
	}

	return conn;
}

void mysql_mem_close(MYSQL *conn)
{
	if (conn) {
		mysql_close(conn);
	}

	conn = NULL;
}

/*
 *   count to be unsigned long. FIXME
 */
IMEI_USER *mysql_mem_one_load(MYSQL *conn, IMEI_USER **mem, int *count, char *id)
{
	unsigned long   i;
	unsigned long   num_fields;
	unsigned long   num_rows;
	char            sql[SQL_LEN] = {};
	IMEI_USER       *imei_user = NULL;
	MYSQL_RES       *result;
	MYSQL_ROW       row;

	snprintf(sql, sizeof(sql), "select imei from %s where businessID='%s'", g_mysql_info.table, id);

	if (mysql_real_query(conn, sql, strlen(sql))) {
		log_info(LOG_E, "error:mysql_real_query,%s\n", mysql_error(conn));
		return NULL;
	}

	result = mysql_store_result(conn);

	if (!result) {
		log_info(LOG_E, "error:mysql_store_result,%s\n", mysql_error(conn));
		return NULL;
	}

	num_rows = mysql_num_rows(result);
	num_fields = mysql_num_fields(result);

	if (1LU != num_fields) {
		mysql_free_result(result);
		log_info(LOG_E, "num_fields != 1\n");
		return NULL;
	}

	imei_user = (IMEI_USER *)realloc(*mem, (*count + num_rows) * sizeof(IMEI_USER));

	if (!imei_user) {
		mysql_free_result(result);
		log_info(LOG_E, "realloc error\n");
		return NULL;
	}

	*mem = imei_user;

	if (!num_rows) {/*放在realloc后，防止第一个id出现此种情况，导致*mem==NULL；进而外部判断为错误*/
		log_info(LOG_I, "num_rows == 0\n");
		mysql_free_result(result);
		return *mem;
	}

	for (i = 0LU; i < num_rows; i++) {
		row = mysql_fetch_row(result);
		strcpy((*mem + *count + i)->imei, row[0]);
		strcpy((*mem + *count + i)->user, id);
	}

	mysql_free_result(result);
	*count += num_rows;
	dprintf(1, "businessID[%s]imei加载数量:[%lu]\n", id, num_rows);
	return *mem;
}

int mysql_mem_all_load(void)
{
	int             i;
	int             count = 0;
	MYSQL           *conn;
	IMEI_USER       *imei_user = NULL;
	IMEI_USER       **mem = &imei_user;
	time_t          tm;
	char            tm_buf[32] = {};
	char            *tm_fmt = "%Y%m%d-%H%M%S";

	conn = mysql_mem_connect();

	if (!conn) {
		return -1;
	}

	for (i = 0; i < g_mysql_imei_user.cnt; i++) {
		if (NULL == mysql_mem_one_load(conn, mem, &count, g_mysql_imei_user.ptr_tab[i])) {
			if (*mem) {
				free(*mem), *mem = NULL;
			}

			mysql_mem_close(conn);
			return -1;
		}
	}

	mysql_mem_close(conn);

	qsort(imei_user, count, sizeof(IMEI_USER), imei_compare);

	/*lock*/
	pthread_rwlock_wrlock(&rwlock);

	if (g_imei_tab) {
		free(g_imei_tab);
	}

	g_imei_tab = imei_user;
	g_imei_cnt = count;
	pthread_rwlock_unlock(&rwlock);
	/*unlock*/

	time(&tm);
	strftime(tm_buf, sizeof(tm_buf), tm_fmt, localtime(&tm));
	dprintf(1, "[%s]---------------->加载总数量:[%d]<----------------\n", tm_buf, count);
	return 0;
}

int imei_init(void)
{
	pthread_rwlock_init(&rwlock, NULL);

	worker_timer_do = mysql_mem_all_load;

	if (-1 == mysql_info_init()) {
		return -1;
	}

	if (-1 == mysql_mem_all_load()) {
		return -1;
	}

#if 0
	int i;

	for (i = 0; i < g_imei_cnt; i++) {
		dprintf(1, "imei[%s],id[%s]\n", g_imei_tab[i].imei, g_imei_tab[i].user);
		log_info(LOG_S, "imei[%s],id[%s]\n", g_imei_tab[i].imei, g_imei_tab[i].user);
	}
#endif
	return 0;
}

void imei_term(void)
{
	g_imei_tab && (free(g_imei_tab), 1);
	pthread_rwlock_destroy(&rwlock);
}

int user_search(char *imei, char *user)
{
	IMEI_USER       key = {};
	IMEI_USER       *val = NULL;

	strcpy(key.imei, imei);
	/*lock*/
	pthread_rwlock_rdlock(&rwlock);
	val = bsearch(&key, g_imei_tab, g_imei_cnt, sizeof(IMEI_USER), imei_compare);
	pthread_rwlock_unlock(&rwlock);

	if (NULL == val) {
		return -1;
	}

	/*unlock*/

	strcpy(user, val->user);
	return 0;
}

#if 0

int main(void)
{
	int     i;
	char    imei[16] = {};
	char    user[32] = {};

	imei_init();

	for (i = 0; i < g_imei_cnt; i++) {
		printf("imei[%s],id[%s]\n", g_imei_tab[i].imei, g_imei_tab[i].user);
	}

	scanf("%s", imei);
	user_search(imei, user);
	printf("user:%s\n", user);

	imei_term();
	return 0;
}
#endif	/* if 0 */

