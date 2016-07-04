#include <string.h>
#include <stdlib.h>

#include "redis_api/redis_status.h"
#include "base/utils.h"
#include "tsdb_cfg.h"
#include "tsdb_ldb.h"

#include "binlog.h"
#include "slave.h"
#include "ldb_priv.h"

#define OPT_NUMBER      ":%d\r\n"
#define OPT_PONG        "+PONG\r\n"
#define OPT_NULL        "$-1\r\n"
#define OPT_EMPTY_MULTI "*0\r\n"

static struct _leveldb_stuff *s_ldb = NULL;

int G_PAGE_SIZE = 0;

short   LDB_READONLY_SWITCH;
time_t  LDB_START_TIME;

int tsdb_ldb_init(char *db_name, struct tsdb_cfg_file *p_cfg)
{
	char temp[1024] = { 0 };

	if ((NULL == db_name) || ('\0' == db_name[0]) || (NULL == p_cfg)) {
		return -1;
	}

	if (NULL != s_ldb) {
		return 0;
	}

	time(&LDB_START_TIME);

	G_PAGE_SIZE = getpagesize();

	LDB_READONLY_SWITCH = p_cfg->ldb_readonly_switch;

	snprintf(temp, sizeof(temp), "%s/%s", p_cfg->work_path, db_name);

	s_ldb = ldb_initialize(temp, p_cfg->ldb_block_size, p_cfg->ldb_write_buffer_size, p_cfg->ldb_cache_lru_size, p_cfg->ldb_bloom_key_size, p_cfg->ldb_compaction_speed);

	if (NULL == s_ldb) {
		x_printf(F, "ldb_initialize failed!");
		return -1;
	}

	/* initial binlog. */
	binlog_open(s_ldb, p_cfg->has_slave);

	/* initial slave. */
	if (slave_open(s_ldb, p_cfg->has_slave, p_cfg->slave_ip, p_cfg->slave_wport) != 0) {
		binlog_close();
		ldb_close(s_ldb);
		s_ldb = NULL;
		return -1;
	}

	return 0;
}

int tsdb_ldb_close(void)
{
	x_printf(I, "close ldb.");

	slave_close();

	binlog_close();

	ldb_close(s_ldb);
	s_ldb = NULL;

	return 0;
}

int tsdb_ldb_set(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->fields != 2)) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_put(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, p_buf + p_rst->field[1].offset, p_rst->field[1].len);

	if ((ok == 0) || (ok == 1)) {
		cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_del(struct data_node *p_node)
{
	char                    ret_num[16] = { 0 };
	int                     i = 0;
	int                     ok = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->fields < 1)) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	for (i = 0; i < p_rst->fields; ++i) {
		ok = binlog_batch_delete(s_ldb, p_buf + p_rst->field[i].offset, p_rst->field[i].len);

		if (ok < 0) {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	ok = binlog_batch_commit(s_ldb);

	if (ok < 0) {
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	sprintf(ret_num, OPT_NUMBER, p_rst->fields);
	cache_append(&p_node->mdl_send.cache, ret_num, strlen(ret_num));

	return X_DONE_OK;
}

int tsdb_ldb_mset(struct data_node *p_node)
{
	int                     i = 0;
	int                     ok = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->fields < 1) || (p_rst->fields % 2 != 0)) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	for (i = 0; i < p_rst->fields; i = i + 2) {
		ok = binlog_batch_put(s_ldb, p_buf + p_rst->field[i].offset, p_rst->field[i].len, p_buf + p_rst->field[i + 1].offset, p_rst->field[i + 1].len);

		if (ok < 0) {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	ok = binlog_batch_commit(s_ldb);

	if (ok < 0) {
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));

	return X_DONE_OK;
}

int tsdb_ldb_sadd(struct data_node *p_node)
{
	int                     ok = 0;
        char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
        struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;
	//struct redis_status     *r_rst = (struct redis_status     *)malloc(sizeof(struct redis_status));
	char                    *result = NULL;
	int			size = 0;
	char			*new_str = NULL;
	int			idx;
	bool			has_set_data = false;	

        if ((LDB_READONLY_SWITCH == 1) || (p_rst->fields < 2)) {
                cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
                return X_EXECUTE_ERROR;
        }
	for (idx = 1; idx < p_rst->fields; idx++) {
	has_set_data = false;
	char target_str[p_rst->field[idx].len + 1];
	memset(target_str, 0, p_rst->field[idx].len + 1);
        strncpy(target_str, p_buf + p_rst->field[idx].offset, p_rst->field[idx].len);
        printf("The target_str = %s\n", target_str);	

	result = ldb_get(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, &size);
	if (NULL != result) {
		char str[strlen(result) +1];
		memset(str, 0, strlen(result) +1);
		strncpy(str, result, strlen(result));
		printf("The str = %s\n", str);
		if (str && target_str) {
			char *p = strtok(result, "@");
			if (!p) {
				printf("The data in tsdb can't be sadd\n");
			}		
			p = strtok(NULL, "|");
			while(p!=NULL) {
				printf("%s\n",p);
				if(strcmp(target_str, p) == 0) {
					printf("Had set the data.\n");
					//cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
					//return X_DONE_OK;
					has_set_data = true;
					break;
				}
				p=strtok(NULL,"|");
			}

			if (has_set_data) {
				continue;
			}

			new_str = (char *)malloc(strlen(str) + strlen(target_str) + 2); // "\0" 和"|", head '*'前的数字可能进位
			memset(new_str, 0, strlen(str) + strlen(target_str) + 2);
			strcpy(new_str, str);  
			strcat(new_str, target_str);
			strcat(new_str, "|");
			printf("new_str is %s\n", new_str);
		}
		else {
			printf("str is NULL or target_str is NULL\n");
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}
	else{
		if(target_str) {
			char *head = "0*1@";
			new_str = (char*)malloc(strlen(head) + strlen(target_str) + 2);
			memset(new_str, 0, strlen(head) + strlen(target_str) + 2);
			strcpy(new_str, head);
			strcat(new_str, target_str);
			strcat(new_str, "|");
			printf("new_str is %s\n", new_str);
		}
		else {
			printf("target_str is NULL or target_str is NULL\n");
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
                        return X_EXECUTE_ERROR;
		}
	}

	//Update head.
	char *string = (char*)malloc(strlen(new_str) + 2);
	memset(string, 0, strlen(new_str) + 2);
	char *n = strchr(new_str, '*');
	char str_tmp[strlen(n) + 1];
	strcpy(str_tmp, n);
	printf("* addr is %s\n", n);
	char *r = strtok(new_str, "*");
        if (r) {
        	sprintf(string, "%d%s",atoi(r) + 1, str_tmp);
        }
	
	printf("string = %s\n", string);

	ok = binlog_put(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, string, strlen(string));

        if ((ok == 0) || (ok == 1)) {
                //cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
		free(new_str);
		free(string);
                //return X_DONE_OK;
        } else {
                cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		free(new_str);
		free(string);
                return X_EXECUTE_ERROR;
        }
	}
	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
	return X_DONE_OK;
}

int tsdb_ldb_get(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	int                     ret = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	result = ldb_get(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, &size);

	if (NULL != result) {
		char tmp[32] = { 0 };

		if (size == 0) {
			free(result);
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}

		sprintf(tmp, "$%d\r\n", size);
		ret = cache_append(&p_node->mdl_send.cache, tmp, strlen(tmp));
		ret = cache_append(&p_node->mdl_send.cache, result, size);
		free(result);

		if (ret == X_MALLOC_FAILED) {
			cache_clean(&p_node->mdl_send.cache);
			cache_append(&p_node->mdl_send.cache, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
			return X_MALLOC_FAILED;
		}

		ret = cache_append(&p_node->mdl_send.cache, "\r\n", 2);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_append(&p_node->mdl_send.cache, OPT_NULL, strlen(OPT_NULL));
			return X_DONE_OK;
		} else {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_lrange(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields != 3) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_lrangeget(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, p_buf + p_rst->field[1].offset, p_rst->field[1].len,
			p_buf + p_rst->field[2].offset, p_rst->field[2].len, &size);

	if (result) {
		cache_append(&p_node->mdl_send.cache, result, size);
		free(result);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_append(&p_node->mdl_send.cache, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_keys(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields != 1) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_keys(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, &size);

	if (result) {
		cache_append(&p_node->mdl_send.cache, result, size);
		free(result);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_append(&p_node->mdl_send.cache, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_values(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields != 1) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_values(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, &size);

	if (result) {
		cache_append(&p_node->mdl_send.cache, result, size);
		free(result);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_append(&p_node->mdl_send.cache, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_info(struct data_node *p_node)
{
	char    *result = NULL;
	int     size = 0;

	result = ldb_info(s_ldb, &size);

	if (result) {
		cache_append(&p_node->mdl_send.cache, result, size);
		free(result);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_append(&p_node->mdl_send.cache, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_ping(struct data_node *p_node)
{
	cache_append(&p_node->mdl_send.cache, OPT_PONG, strlen(OPT_PONG));

	return 0;
}

int tsdb_ldb_exists(struct data_node *p_node)
{
	int                     ok = 0;
	char                    ret_num[16] = { 0 };
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	ok = ldb_exists(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len);

	sprintf(ret_num, OPT_NUMBER, ok);
	cache_append(&p_node->mdl_send.cache, ret_num, strlen(ret_num));

	return 0;
}

int tsdb_ldb_syncset(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields != 2) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_syncset(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len, p_buf + p_rst->field[1].offset, p_rst->field[1].len);

	if ((ok == 0) || (ok == 1)) {
		cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_syncdel(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields < 1) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_syncdel(s_ldb, p_buf + p_rst->field[0].offset, p_rst->field[0].len);

	if ((ok == 0) || (ok == 1)) {
		cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_compact(struct data_node *p_node)
{
	ldb_compact(s_ldb);

	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));

	return X_DONE_OK;
}

