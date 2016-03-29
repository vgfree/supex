#pragma once

#include "x_utils.h"
#include "mfptp_cfg.h"

void load_mfptp_cfg_argv(struct mfptp_cfg_argv *p_cfg, int argc, char **argv);

void load_mfptp_cfg_file(struct mfptp_cfg_file *p_cfg, char *name);

// 从配置文件中读取各个模块是否开启，在配置文件里面计算module的值
struct log_module_s
{
	unsigned int    log_json_conf;
	unsigned int    log_init;
	unsigned int    log_net_conn;
	unsigned int    log_mfptp_auth;
	unsigned int    log_mfptp_parse;
	unsigned int    log_net_uplink;

	unsigned int    log_mfptp_pack;
	unsigned int    log_net_downlink;
	unsigned int    log_all;
	unsigned int    log_none;
	unsigned int    log_to_redis;
	unsigned int    log_timer;
};
// 读取日志配置文件
void load_log_cfg_file(struct file_log_s *p_file_log, char *name);

