#pragma once

#include "libevcs.h"

#include "tsdb_cfg.h"

#define LDB_DB_NAME "data"

int tsdb_ldb_init(char *db_name, struct tsdb_cfg_file *p_cfg);

int tsdb_ldb_close(void);

int tsdb_ldb_set(struct data_node *p_node);

int tsdb_ldb_del(struct data_node *p_node);

int tsdb_ldb_mset(struct data_node *p_node);

int tsdb_ldb_sadd(struct data_node *p_node);

int tsdb_ldb_get(struct data_node *p_node);

int tsdb_ldb_lrange(struct data_node *p_node);

int tsdb_ldb_keys(struct data_node *p_node);

int tsdb_ldb_values(struct data_node *p_node);

int tsdb_ldb_info(struct data_node *p_node);

int tsdb_ldb_ping(struct data_node *p_node);

int tsdb_ldb_exists(struct data_node *p_node);

int tsdb_ldb_syncset(struct data_node *p_node);

int tsdb_ldb_syncdel(struct data_node *p_node);

int tsdb_ldb_compact(struct data_node *p_node);

