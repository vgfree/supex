#pragma once

#include "libevcs.h"

#include "tsdb_cfg.h"

int tsdb_kv_init(char *ident);

int tsdb_kv_close(void);

int tsdb_kv_set(struct data_node *p_node);

int tsdb_kv_sadd(struct data_node *p_node);

int tsdb_kv_del(struct data_node *p_node);

int tsdb_kv_mset(struct data_node *p_node);

int tsdb_kv_get(struct data_node *p_node);

