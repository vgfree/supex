#pragma once

#include "major_def.h"

#include "tsdb_cfg.h"

int tsdb_kv_init(void);

int tsdb_kv_close(void);

int tsdb_kv_set(struct data_node *p_node);

int tsdb_kv_del(struct data_node *p_node);

int tsdb_kv_mset(struct data_node *p_node);

int tsdb_kv_get(struct data_node *p_node);

