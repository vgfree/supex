/*
 * file: zk.h
 * date: 2014-08-04
 * auth: chenjianfei@daoke.me
 * desc: zk.h
 */

#pragma once

#include <string.h>
#include <stdlib.h>		// FIXME: must include, else atol convert error.
#include <stdint.h>
#include <limits.h>

#include "zookeeper_log.h"
#include "zookeeper.h"

#define EXPANSION_SIZE          128
#define HOST_LEN                64
#define DN_PER_DS               2	// data nodes count per data set.
#define MAX_KS_PER_EXPANSION    32	// max keyset count per expansion
#define MAX_DS_PER_KEY          2
#define ERR_MSG_LEN             64

// role
#define MASTER                  "MASTER"
#define SLAVE                   "SLAVE"
#define ERRROLE                 "ERRROLE"

// cache structure.
typedef enum
{
	NA = 0,		/* not available. */
	RO = 1,		/* read only. */
	RW = 2,		/* readable and writable. */
} tmode_t;

struct _data_set;
struct _key_set;
struct _expansion_set;
struct _ctx_t;

typedef struct _data_node
{
	char                    ip[16];
	uint32_t                w_port;
	uint32_t                r_port;
	const char              *role;
	struct _data_set        *parent;
	void                    *ctx;
} data_node_t;

typedef struct _data_set
{
	char            path[NAME_MAX];		/* dataset name: DS1 */
	uint32_t        id;
	tmode_t         mode;
	uint64_t        s_time;			/* start time: 20120101000000 */
	uint64_t        e_time;			/* end time: 20120101000000 */
	uint8_t         dn_cnt;			/* data node count. */
	data_node_t     data_node[DN_PER_DS];	/* data node array. */
	uint8_t         dn_idx;
	struct _key_set *parent;
} data_set_t;

typedef struct _key_set
{
	char                    path[NAME_MAX];			/* "/keys/0:2048" */
	uint8_t                 is_synced;			/* 0: need refresh; 1: not need refresh. */
	uint64_t                s_key;				/* start key: 0 */
	uint64_t                e_key;				/* end key: 2048 */
	uint8_t                 ds_cnt;				/* dataset count. */
	data_set_t              data_set[MAX_DS_PER_KEY];	/* dataset array: data_set[0] <--> read/write; data_set[>0] <--> readonly. */
	struct _expansion_set   *parent;
} key_set_t;

typedef struct _expansion_set
{
	char            path[NAME_MAX];
	uint8_t         is_synced;
	tmode_t         mode;
	uint64_t        s_time;
	uint64_t        e_time;
	uint8_t         key_cnt;
	key_set_t       key_set[MAX_KS_PER_EXPANSION];
	struct _ctx     *parent;
} expansion_set_t;

typedef void (*alloc_dn_hook_f)(void *ctx, data_node_t *dn);
typedef void (*free_dn_hook_f)(void *ctx, data_node_t *dn);

typedef struct _ctx
{
	char            zkhost[NAME_MAX];
	zhandle_t       *zkhandle;
	uint16_t        expansion_cnt;
	uint16_t        expansion_rw_idx;
	expansion_set_t expansion_set[EXPANSION_SIZE];
	alloc_dn_hook_f alloc_dn_hook;
	free_dn_hook_f  free_dn_hook;
	void            *ctx;
} zk_ctx_t;

/* apis. */
zk_ctx_t *open_zkhandler(const char *zkhost, alloc_dn_hook_f alloc_dn_hook, free_dn_hook_f free_dn_hook, void *ctx);

int close_zkhandler(zk_ctx_t *zc);

int register_zkcache(zk_ctx_t *zc, const char *rnode);

int unregister_zkcache(zk_ctx_t *zc);

int get_write_dataset(zk_ctx_t *zc, uint64_t key_field, const data_set_t **ds);

int get_read_dataset(zk_ctx_t *zc, uint64_t key_field, uint64_t time_field, const data_set_t **ds);

