#pragma once

#include <stdint.h>
#include "topo_cfg.h"

#define TOPO_CONN_TYPE_NO       0		// 无连接
#define TOPO_CONN_TYPE_DIRECT   1		// 道路直接相连
#define TOPO_CONN_TYPE_BYROAD   2		// 道路通过辅路节点相连
#define TOPO_CONN_TYPE_INDIRECT 3		// 道路通过中间道路相连

typedef struct topo_file_line_t
{
	uint8_t         sgid1;
	uint8_t         count1;
	uint8_t         sgid2;
	uint8_t         count2;
	uint8_t         conn_count;
	uint8_t         conn_type;
	uint32_t        id;
	uint32_t        rrid1;
	uint32_t        rrid2;
	double          SL;
	double          SB;
	double          EL;
	double          EB;
} topo_file_line_t;

typedef struct topo_node_t
{
	double  L1;
	double  B1;
	double  L2;
	double  B2;
} topo_node_t;

typedef struct topo_conn_sg_t
{
	uint8_t         sgid;
	uint8_t         conn_type;
	uint32_t        rrid;
	topo_node_t     conn_node;
} topo_conn_sg_t;

typedef struct topo_info_t
{
	uint32_t        rrid;
	uint8_t         sgid;
	uint8_t         conn_count;
	uint8_t         max_count;
	topo_conn_sg_t  *p_conn_buff;
} topo_info_t;

typedef struct topo_info_rr_t
{
	uint32_t        rrid;
	uint8_t         sg_count;
	topo_info_t     *p_topo_buf;
} topo_info_rr_t;

typedef struct map_topo_manager_t
{
	uint32_t        max_rr_id;
	uint16_t        load_once;
	uint16_t        rrid_buff_len;
	topo_info_rr_t  **p_manage_buff;
	topo_cfg_file_t file_cfg;
} map_topo_manager_t;

int map_topo_manager_init_buff(map_topo_manager_t *p_manage, uint16_t load_once, uint32_t max_rrid);

topo_info_t *map_topo_get_conn(map_topo_manager_t *p_manage, uint32_t rrid, uint8_t sgid);

int map_topo_manager_add_line(map_topo_manager_t *p_manager, topo_file_line_t *p_line);

