#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_mfptp_cfg.h"
#include "json.h"

void load_mfptp_cfg_argv(struct mfptp_cfg_argv *p_cfg, int argc, char **argv)
{
	LOG(LOG_JSON_CONF, M, "load mfptp configure file start!\n");
	strncat(p_cfg->msmq_name, strrchr(argv[0], 0x2f), MAX_FILE_NAME_SIZE - 1);
	strncat(p_cfg->serv_name, (strrchr(argv[0], 0x2f) + 1), MAX_FILE_NAME_SIZE - 1);
	char    opt;
	short   cfg = false;

	while ((opt = getopt(argc, argv, "c:h:p")) != -1) {
		switch (opt)
		{
			case 'c':
				cfg = true;
				strncat(p_cfg->conf_name, optarg, MAX_FILE_NAME_SIZE - 1);
				printf("load config:'%s'\n", optarg);
				LOG(LOG_JSON_CONF, I, "load config:'%s'\n", optarg);
				break;

			default:
				printf("load config,unknow option:'%c'\n", opt);
				LOG(LOG_JSON_CONF, E, "unknow option :%c\n", opt);
				LOG(LOG_JSON_CONF, E, "\x1B[0;32mUSE LIKE :\n\x1B[1;31m\t./%s -c %s_conf.json\n\x1B[m", p_cfg->serv_name, p_cfg->serv_name);
				exit(EXIT_FAILURE);
		}
	}

	if (!cfg) {
		int len = strlen(p_cfg->serv_name);
		strncat(p_cfg->conf_name, p_cfg->serv_name, MAX_FILE_NAME_SIZE - 1);
		strncat(p_cfg->conf_name, "_conf.json", MAX_FILE_NAME_SIZE - len - 1);
	}
}

void load_mfptp_cfg_file(struct mfptp_cfg_file *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "mfptp_port", &obj)) {
		p_cfg->srv_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_front_port", &obj)) {
		p_cfg->front_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_back_port", &obj)) {
		p_cfg->back_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_usr_msg_port", &obj)) {
		p_cfg->usr_msg_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_gp_msg_port", &obj)) {
		p_cfg->gp_msg_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_edit_user_info_port", &obj)) {
		p_cfg->edit_user_info_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "worker_counts", &obj)) {
		p_cfg->worker_counts = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_redis_address", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->redis_address = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_redis_port", &obj)) {
		p_cfg->redis_port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "mfptp_user_online_status", &obj)) {
		p_cfg->user_online_status = (short)json_object_get_int(obj);
	} else {
		p_cfg->user_online_status = 0;
	}

	if (json_object_object_get_ex(cfg, "max_req_size", &obj)) {
		p_cfg->max_req_size = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_file = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else { goto fail; }

	LOG(LOG_JSON_CONF, M, " load mfptp configure file OK!\n\n");
	return;

fail:
	x_printf(D, "无效的mfptpServer配置文件 :%s\n", name);
	LOG(LOG_JSON_CONF, F, "invalid mfptp configure file :%s\n", name);
	exit(EXIT_FAILURE);
}

/*
 *功能：加载日志配置文件,获取变量log_level/log_module---需要计算
 *参数：p_file_log---日志相关结构体
 *        name --- 配置文件名称
 *返回值： void
 **/
void load_log_cfg_file(struct file_log_s *p_file_log, char *name)
{
	struct log_module_s modules = {};

	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		printf("json_object_from_file error :%s\n", name);
		goto fail;
	}

	// 只获取log_level/log_module 两个变量
	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_file_log->level = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_file_log->log_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_file_log->log_filename = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "release_log", &obj)) {
		p_file_log->release_log = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_json_conf", &obj)) {
		modules.log_json_conf = json_object_get_int(obj);
		p_file_log->module |= (modules.log_json_conf << 0);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_init", &obj)) {
		modules.log_init = json_object_get_int(obj);
		p_file_log->module |= (modules.log_init << 1);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_net_conn", &obj)) {
		modules.log_net_conn = json_object_get_int(obj);
		p_file_log->module |= (modules.log_net_conn << 2);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_mfptp_auth", &obj)) {
		modules.log_mfptp_auth = json_object_get_int(obj);
		p_file_log->module |= (modules.log_mfptp_auth << 3);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_mfptp_parse", &obj)) {
		modules.log_mfptp_parse = json_object_get_int(obj);
		p_file_log->module |= (modules.log_mfptp_parse << 4);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_net_uplink", &obj)) {
		modules.log_net_uplink = json_object_get_int(obj);
		p_file_log->module |= (modules.log_net_uplink << 5);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_mfptp_pack", &obj)) {
		modules.log_mfptp_pack |= json_object_get_int(obj);
		p_file_log->module |= (modules.log_mfptp_pack << 6);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_net_downlink", &obj)) {
		modules.log_net_downlink = json_object_get_int(obj);
		p_file_log->module |= (modules.log_net_downlink << 7);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_to_redis", &obj)) {
		modules.log_to_redis = json_object_get_int(obj);
		p_file_log->module |= (modules.log_to_redis << 8);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_timer", &obj)) {
		modules.log_timer = json_object_get_int(obj);
		p_file_log->module |= (modules.log_timer << 9);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_all", &obj)) {
		modules.log_all = json_object_get_int(obj);

		if (modules.log_all == 1) {
			p_file_log->module = 0xffffffff;
		}
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_none", &obj)) {
		modules.log_none = json_object_get_int(obj);

		if (modules.log_none == 1) {
			p_file_log->module = 0;
		}
	} else { goto fail; }

	// 计算module字段

	printf("加载Log配置文件成功----release_log = %d,log_level = %d, log_module = 0x%08x\n", p_file_log->release_log, p_file_log->level, p_file_log->module);
	return;

fail:
	printf("load log configure file failed! :%s\n", name);
	exit(EXIT_FAILURE);
}

