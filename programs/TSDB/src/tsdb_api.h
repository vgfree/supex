#pragma once

#include "major_def.h"

#include "tsdb_cfg.h"

int tsdb_cmd_init(struct tsdb_cfg_file *p_cfg);

int tsdb_cmd_close(void);

int tsdb_cmd_set(struct data_node *p_node);

int tsdb_cmd_del(struct data_node *p_node);

int tsdb_cmd_mset(struct data_node *p_node);

int tsdb_cmd_sadd(struct data_node *p_node);

int tsdb_cmd_get(struct data_node *p_node);

int tsdb_cmd_lrange(struct data_node *p_node);

int tsdb_cmd_keys(struct data_node *p_node);

int tsdb_cmd_values(struct data_node *p_node);

int tsdb_cmd_info(struct data_node *p_node);

int tsdb_cmd_ping(struct data_node *p_node);

int tsdb_cmd_exists(struct data_node *p_node);

int tsdb_cmd_syncset(struct data_node *p_node);

int tsdb_cmd_syncdel(struct data_node *p_node);

int tsdb_cmd_compact(struct data_node *p_node);

