#include "map_topo.h"
#include "topo_info.h"
#include <stdbool.h>
#include "libmini.h"

static map_topo_manager_t g_topo_manage;

int map_topo_init(char *p_file_name)
{
	int ret = topo_cfg_init_file(p_file_name, &(g_topo_manage.file_cfg));

	if (ret != 0) {
		return ret;
	}

	return 0;
}

int map_topo_load()
{
	return topo_file_load(&g_topo_manage);
}

static bool is_connected(uint32_t rrid1, uint8_t sgid1, uint32_t rrid2, uint8_t sgid2, uint8_t deep)
{
	if (deep <= 0) {
		return 0;
	}

	topo_info_t *p_topo = map_topo_get_conn(&g_topo_manage, rrid1, sgid1);

	if (!p_topo) {
		return 0;
	}

	int i;

	for (i = 0; i < p_topo->conn_count; i++) {
		topo_conn_sg_t *p_sg_conn = p_topo->p_conn_buff + i;

		if ((p_sg_conn->rrid == rrid2) || (p_sg_conn->sgid == sgid2)) {
			return true;
		} else {
			if (is_connected(p_sg_conn->rrid, p_sg_conn->sgid, rrid2, sgid2, deep - 1) != 0) {	// 递归调用
				return true;
			}
		}
	}

	return false;
}

static bool map_topo_is_direct_connect(uint32_t rrid1, uint8_t sgid1, uint32_t rrid2, uint8_t sgid2, topo_conn_sg_t **pp_sg_conn)
{
	topo_info_t *p_topo = map_topo_get_conn(&g_topo_manage, rrid1, sgid1);

	if (!p_topo) {
		return 0;
	}

	int i;

	// 先判断是否直接连接
	for (i = 0; i < p_topo->conn_count; i++) {
		topo_conn_sg_t *p_sg_conn = p_topo->p_conn_buff + i;

		if ((p_sg_conn->rrid == rrid2) && (p_sg_conn->sgid == sgid2)) {
			*pp_sg_conn = p_sg_conn;
			return true;
		}
	}

	return false;
}

int map_topo_isconnect(uint32_t rrid1, uint8_t sgid1, uint32_t rrid2, uint8_t sgid2, uint8_t deep, topo_node_t *p_node)
{
	// 先判断是否直接先连，返回连接节点
	topo_conn_sg_t  *p_sg_conn;
	bool            direct_conn = map_topo_is_direct_connect(rrid1, sgid1, rrid2, sgid2, &p_sg_conn);

	if (direct_conn) {
		if (p_sg_conn->conn_type == TOPO_CONN_TYPE_DIRECT) {
			return TOPO_CONN_TYPE_DIRECT;
		} else if (p_sg_conn->conn_type == TOPO_CONN_TYPE_BYROAD) {
			p_node->B1 = p_sg_conn->conn_node.B1;
			p_node->L1 = p_sg_conn->conn_node.L2;
			p_node->B2 = p_sg_conn->conn_node.B2;
			p_node->L2 = p_sg_conn->conn_node.L2;

			return TOPO_CONN_TYPE_BYROAD;
		} else {
			x_printf(E, "conn type error!");
		}
	}

	if (is_connected(rrid1, sgid1, rrid2, sgid2, deep)) {
		return TOPO_CONN_TYPE_INDIRECT;
	}

	return TOPO_CONN_TYPE_NO;
}

void map_topo_destory()
{}

