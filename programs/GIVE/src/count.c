/*
 * Author       : chenzutao
 * Date         : 2015-11-20
 * Function     : count gps data, include gps numbers and stream size
 */

// TODO: create MYSQL table

#include "count.h"
#include "rr_cfg.h"

extern struct rr_cfg_file g_rr_cfg_file;
// #define __TEST 1

static int str_cmd_in(kv_handler_t *handler, struct tm *timenow, char *const_str, unsigned int city_code, unsigned int val)
{
	char cmd[DAT_BUF_SIZE] = { '\0' };

	snprintf(cmd, DAT_BUF_SIZE, "INCRBY %u:%d%02d%02d%02d%d:%s %u", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60), const_str, val);

	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		// x_x_printf(E, E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	kv_answer_release(ans);

#if __TEST
	snprintf(cmd, DAT_BUF_SIZE, "GET %u:%d%02d%02d%02d%d:%s", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60), const_str);
	ans = kv_ask(handler, cmd, strlen(cmd));
	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], %s:%lld\n", cmd, const_str, atoll((char *)value->ptr));
		} else {
			x_printf(E, "Failed to get gpsCount by cmd[%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	}
	kv_answer_release(ans);
#endif
	return DAT_OK;
}

static int set_cmd_in(kv_handler_t *handler, struct tm *timenow, char *const_str, unsigned int city_code, unsigned long long val)
{
	char cmd[DAT_BUF_SIZE] = { '\0' };

	snprintf(cmd, DAT_BUF_SIZE, "SADD %u:%d%02d%02d%02d%d:%s %llu", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60), const_str, val);

	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	kv_answer_release(ans);

#if __TEST
	snprintf(cmd, DAT_BUF_SIZE, "SCARD %u:%d%02d%02d%02d%d:%s", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60)), const_str);
	ans = kv_ask(handler, cmd, strlen(cmd));
	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], %s:%lld\n", cmd, const_str, atoll((char *)value->ptr));
		} else {
			x_printf(E, "Failed to get activeUser count by [%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	}
	kv_answer_release(ans);
#endif
	return DAT_OK;
}

static int str_cmd_out(kv_handler_t *handler, const char *const_str, unsigned long long *size, struct tm *timenow, unsigned int city_code)
{
	if (!handler || !const_str) {
		return DAT_ERR;
	}

	char cmd[DAT_BUF_SIZE] = { 0 };
	snprintf(cmd, DAT_BUF_SIZE, "GET %u:%d%02d%02d%02d%d:%s", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60))	/*tm_min*/, const_str);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], gpsCount:%lld\n", cmd, atoll((char *)value->ptr));
			*size = atoll((char *)value->ptr);
		} else {
			x_printf(E, "Failed to get gpsCount by cmd[%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	}

	kv_answer_release(ans);
	return DAT_OK;
}

static int set_cmd_out(kv_handler_t *handler, const char *const_str, unsigned long long *size, struct tm *timenow, unsigned int city_code)
{
	char cmd[DAT_BUF_SIZE] = { '\0' };

	snprintf(cmd, DAT_BUF_SIZE, "SCARD %u:%d%02d%02d%02d%d:%s", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60)), const_str);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], %s:%lld\n", cmd, const_str, atoll((char *)value->ptr));
			*size = atoll((char *)value->ptr);
		} else {
			x_printf(E, "Failed to get activeUser count by [%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	}

	kv_answer_release(ans);
	return DAT_OK;
}

static int add_active_city(kv_handler_t *handler, struct tm *timenow, unsigned int city_code)
{
	char cmd[DAT_BUF_SIZE] = { '\0' };

	snprintf(cmd, DAT_BUF_SIZE, "SADD %d%02d%02d%02d%d:activeCity %u", timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60), city_code);

	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	kv_answer_release(ans);

#if __TEST
	snprintf(cmd, DAT_BUF_SIZE, "SCARD %d%02d%02d%02d%d:activeCity", timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60));
	ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], activeCity:%lld\n", cmd, atoll((char *)value->ptr));
		} else {
			x_printf(E, "Failed to get activeUser count by [%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	} else if (len > 1) {
		int                     i = 0;
		kv_answer_value_t       *value;
		kv_answer_iter_t        *iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);

		while ((value = kv_answer_next(iter)) != NULL) {
			x_printf(D, "%dth cityCode:%d\n", i, atoi((char *)value->ptr));
		}

		kv_answer_release_iter(iter);
	}
	kv_answer_release(ans);
#endif	/* if __TEST */
	return DAT_OK;
}

static int get_active_city(kv_handler_t *handler, struct tm *timenow, unsigned int *active_city)
{
	char cmd[DAT_BUF_SIZE] = { '\0' };

	snprintf(cmd, DAT_BUF_SIZE, "SMEMBERS %d%02d%02d%02d%d:activeCity", timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min / (g_rr_cfg_file.synctime / 60));
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	unsigned long   len = kv_answer_length(ans);
	int             cnt = 0;
	int             code = 0;

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			x_printf(I, "after executed cmd[%s], activeCity:%d\n", cmd, atoi((char *)value->ptr));
			cnt = 1;
			active_city[0] = atoi((char *)value->ptr);
			kv_answer_release(ans);
			return cnt;
		} else {
			x_printf(E, "Failed to get activeUser count by [%s].\n", cmd);
			kv_answer_release(ans);
			return DAT_ERR;
		}
	} else if (len > 1) {
		kv_answer_value_t       *value;
		kv_answer_iter_t        *iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);

		while ((value = kv_answer_next(iter)) != NULL) {
			code = atoi((char *)value->ptr);
			x_printf(D, "%dth cityCode:%d\n", cnt, code);
			active_city[cnt] = code;
			cnt++;
		}

		kv_answer_release_iter(iter);
	}

	kv_answer_release(ans);
	return cnt;
	// return DAT_OK;
}

static int str_cmd_del(kv_handler_t *handler, const char *const_str, struct tm *timenow, unsigned int city_code)
{
	if (!handler || !const_str) {
		return DAT_ERR;
	}

	char cmd[DAT_BUF_SIZE] = { 0 };
	snprintf(cmd, DAT_BUF_SIZE, "DEL %u:%d%02d%02d%02d%d:%s", city_code, timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60)), const_str);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	kv_answer_release(ans);
	return DAT_OK;
}

static int city_key_rm(kv_handler_t *handler, struct tm *timenow)
{
	if (!handler || !timenow) {
		return DAT_ERR;
	}

	char cmd[DAT_BUF_SIZE] = { 0 };
	snprintf(cmd, DAT_BUF_SIZE, "DEL %d%02d%02d%02d%d:activeCity", timenow->tm_year + 1900,
		timenow->tm_mon + 1, timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60)));
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
		kv_answer_release(ans);
		x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
		return DAT_ERR;
	}

	kv_answer_release(ans);
	return DAT_OK;
}

int city_data_count(kv_handler_t *handler, data_count_t *data)
{
	if (!handler || !data) {
		x_printf(E, "handler or data is null .\n");
		return DAT_ERR;
	}

	time_t now;
	time(&now);
	struct tm       tm_result = { 0 };
	struct tm       *timenow = localtime_r(&now, &tm_result);

	if (str_cmd_in(handler, timenow, "gpsCount", data->city_code, data->data_cnt) != DAT_OK) {
		x_printf(E, "[** FAILED **] Failed to INCR gpsCount .\n");
	}

	if (str_cmd_in(handler, timenow, "gpsSize", data->city_code, data->data_size) != DAT_OK) {
		x_printf(E, "[** FAILED **] Failed to INCR gpsSize .\n");
	}

	if (set_cmd_in(handler, timenow, "activeUser", data->city_code, data->IMEI) != DAT_OK) {
		x_printf(E, "[** FAILED **] Failed to ADD IMEI[%llu] .\n", data->IMEI);
	}

	if (add_active_city(handler, timenow, data->city_code) != DAT_OK) {
		x_printf(E, "[** FAILED **] Failed to ADD citycode[%u] .\n", data->city_code);
	}

	return DAT_OK;
}

int data_dump(kv_handler_t *handler, sql_conf_t *sql_conf)
{
	if (!handler || !sql_conf) {
		x_printf(E, "handler or sql_conf is null .\n");
		return DAT_ERR;
	}

	if (mysql_library_init(0, NULL, NULL)) {
		x_printf(E, "could not initialize MYSQL library .\n");
		return DAT_ERR;
	}

	MYSQL conn;

	mysql_init(&conn);

	if (!mysql_real_connect(&conn,
		(sql_conf->host) ? sql_conf->host : "localhost",
		sql_conf->username,
		sql_conf->password,
		sql_conf->database,
		sql_conf->port,
		NULL, 0
		)) {
		x_printf(S, "[** FAILED **]Failed to connect to [%s:%d]\n", sql_conf->host ? sql_conf->host : "localhost", sql_conf->port);

		if (mysql_errno(&conn)) {
			x_printf(S, "connection error:%d -> %s .\n", mysql_errno(&conn), mysql_error(&conn));
		}

		mysql_close(&conn);
		mysql_library_end();
		return DAT_ERR;
	}

	// hint: we count every ten minutes and then dump to mysql. when dumping, and current time is 16:00:00, the
	// key time we should dump is: 15:50:00 to 15:59:59. That mean, the key is: 310000:20151119155:gpsCount
	time_t now;
	time(&now);
	// now -= DAT_TIME_INTERVAL;
	now -= g_rr_cfg_file.synctime;
	struct tm       tm_result = { 0 };
	struct tm       *timenow = localtime_r(&now, &tm_result);	// localtime_r() if the safe version of localtime()

	unsigned int    active_city[CITY_SIZE] = { 0 };
	int             active_city_cnt = get_active_city(handler, timenow, active_city);
	int             i;

	for (i = 0; i < active_city_cnt; i++) {
		unsigned long long gps_cnt = 0, gps_size = 0, imei_cnt = 0;

		if (str_cmd_out(handler, "gpsCount", &gps_cnt, timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] get gpsCount from libkv failed, timeInterval:%ld, activeCity:%u.\n", now, active_city[i]);
		}

		if (str_cmd_out(handler, "gpsSize", &gps_size, timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] get gpsSize from libkv failed, timeInterval:%ld, activeCity:%u .\n", now, active_city[i]);
		}

		if (set_cmd_out(handler, "activeUser", &imei_cnt, timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] get imeiCount from libkv failed, timeInterval:%ld, activeCity:%u .\n", now, active_city[i]);
		}

		char sql_str[DAT_SQL_SIZE] = { 0 };
		snprintf(sql_str, DAT_SQL_SIZE, "INSERT INTO %s SET createTime=%ld, intervalTime='%d%02d%02d%02d%02d', gpsCnt=%llu, gpsSize=%llu, imeiCnt=%llu, cityCode=%u",
			sql_conf->table, time(NULL), timenow->tm_year + 1900, timenow->tm_mon + 1,
			timenow->tm_mday, timenow->tm_hour, (int)(timenow->tm_min / (g_rr_cfg_file.synctime / 60)),
			gps_cnt, gps_size, imei_cnt, active_city[i]);

		x_printf(I, "SQL:%s\n", sql_str);

		if (mysql_query(&conn, sql_str)) {
			x_printf(E, "[** FAILED **] SQL:[%s], query error:%s\n", sql_str, mysql_error(&conn));
			mysql_close(&conn);
			mysql_library_end();
			return DAT_ERR;
		}

		// here we should remove data from libkv when dumped into MYSQL
		if (str_cmd_del(handler, "gpsCount", timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] Failed to delete gpsCount from libkv, timeInterval:%ld, activeCity:%u.\n", now, active_city[i]);
		}

		if (str_cmd_del(handler, "gpsSize", timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] Failed to delete gpsSize from libkv , timeInterval:%ld, activeCity:%u.\n", now, active_city[i]);
		}

		if (str_cmd_del(handler, "activeUser", timenow, active_city[i]) != DAT_OK) {
			x_printf(E, "[** FAILED **] Failed to delete activeUser from libkv, timeInterval:%ld, activeCity:%u.\n", now, active_city[i]);
		}

		if (city_key_rm(handler, timenow) != DAT_OK) {
			x_printf(E, "[** FAILED **] Failed to delete activeCity from libkv, timeInterval:%ld.\n", now);
		}
	}

	mysql_close(&conn);
	mysql_library_end();
	return DAT_OK;
}

#if 0
int main(int argc, char **argv)
{
	kv_handler_t    *handler = kv_create(NULL);
	data_count_t    *data = (data_count_t *)calloc(sizeof(data_count_t), 1);

	data->data_size = 76;
	data->data_cnt = 5;
	data->city_code = 310000;
	data->IMEI = 111111111111111;

	sql_conf_t sql_conf = {
		.host           = "127.0.0.1",
		.username       = "root",
		.password       = "abc123",
		.database       = "gpsDataCount",
		.table          = "dataCountInfo",
		.port           = 3306
	};

	int mem = city_data_count(handler, data);

	if (mem < 0) {
		x_printf(E, "KV failed .\n");
	}

	int disk = data_dump(handler, &sql_conf);

	if (disk < 0) {
		x_printf(E, "MYSQL failed .\n");
	}

	kv_destroy(handler);
	free(data);
	return 0;
}
#endif	/* if 0 */

