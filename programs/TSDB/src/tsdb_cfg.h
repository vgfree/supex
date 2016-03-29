#pragma once

#include <stddef.h>

/* node type. */
enum node_type
{
	SINGLE = 1,
	CLUSTER = 2,
};

/* node type. */
enum role_type
{
	MASTER = 1,
	SLAVE = 2,
};

typedef enum TSDB_ENGINE_TYPE
{
	TSDB_ENGINE_UNKNOWN = 0x00,
	TSDB_ENGINE_LDB = 0x01,
	TSDB_ENGINE_KV = 0x02,
	TSDB_ENGINE_MIX = 0x03
} TSDB_ENGINE_e;

struct tsdb_cfg_file
{
	int             node_type;
	char            *mode;					// mode, RO or RW.
	TSDB_ENGINE_e   engine_type;
	int             ds_id;
	long            key_start;
	long            key_end;
	long            start_time;		// time range: start time.
	long            end_time;		// time range: end time.
	char            *zk_server;		// zookeeper server.

	char            *ip;			// IP address.
	short           w_port;			// write port.
	short           r_port;			// read port.

	char            *work_path;		// work space directory.

	short           ldb_readonly_switch;	// read only switch <=> mode
	size_t          ldb_write_buffer_size;
	size_t          ldb_block_size;
	size_t          ldb_cache_lru_size;
	short           ldb_bloom_key_size;
	int             ldb_compaction_speed;

	char            has_slave;		// startup slave thread or not.
	int             role;
	char            *slave_ip;		// slave IP address.
	short           slave_wport;		// slave write port, used for sync.
};

void read_tsdb_cfg(struct tsdb_cfg_file *p_cfg, char *name);

