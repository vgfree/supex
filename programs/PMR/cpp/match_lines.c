#include "match_lines.h"
#include "user_info.h"
#include <stdint.h>
#include <string.h>
#include "libmini.h"
#include "map_topo.h"
#include "map_utils.h"
#include "map_seg.h"
#include "libmini.h"
#include <math.h>
#include "entry.h"

#define MAX_FLAG_SIZE           10
#define TRACK_BACK_DEEP         g_locate_cfg->track_back_deep
#define WEIGHT_DIST_LIMIT       g_locate_cfg->weight_dist_limit
#define NODE_RANGE_DIST         g_locate_cfg->node_range_dist

enum MATCH_FLAG
{
	MATCH_FLAG_NO = 0,
	MATCH_FLAG_LAST_SAM = 1,
	MATCH_FLAG_CONN_DIRECT = 2,
	MATCH_FLAG_CONN_INDIRECT = 3,
	MATCH_FLAG_BYROAD = 4,
	MATCH_FLAG_NEAEST = 5,
	MATCH_FLAG_MIN_SUB = 6,
	MATCH_FLAG_SPEED = 7,
	MATCH_FLAG_ALT = 8,
	MATCH_FLAG_INDIRECT_IN_NODE = 9
};

// static double match_weight[MAX_FLAG_SIZE] = {0, 4.4, 4, 2.5, 2.3, 1.7, 1, 1, 1,2.0};
// static double match_weight[MAX_FLAG_SIZE] = {0, 4.4, 4, 2.0, 2.3, 1.7, 1, 1, 1,2.1};

static char *match_status_name[MAX_FLAG_SIZE] = { "未连接",
						  "相同道路",
						  "直接相连",
						  "间接相连",
						  "辅路加成",
						  "距离加成",
						  "方向角加成",
						  "速度加成",
						  "海拔加成",
						  "间接节点加成" };

static double match_weight(int flag)
{
	double weight = 0;

	switch (flag)
	{
		case MATCH_FLAG_NO:
			weight = g_locate_cfg->weight_no;
			break;

		case MATCH_FLAG_LAST_SAM:
			weight = g_locate_cfg->weight_last_sam;
			break;

		case MATCH_FLAG_CONN_DIRECT:
			weight = g_locate_cfg->weight_conn_direct;
			break;

		case MATCH_FLAG_CONN_INDIRECT:
			weight = g_locate_cfg->weight_conn_indirect;
			break;

		case MATCH_FLAG_BYROAD:
			weight = g_locate_cfg->weight_byroad;
			break;

		case MATCH_FLAG_NEAEST:
			weight = g_locate_cfg->weight_neaest;
			break;

		case MATCH_FLAG_MIN_SUB:
			weight = g_locate_cfg->weight_min_sub;
			break;

		case MATCH_FLAG_SPEED:
			weight = g_locate_cfg->weight_speed;
			break;

		case MATCH_FLAG_ALT:
			weight = g_locate_cfg->weight_alt;
			break;

		case MATCH_FLAG_INDIRECT_IN_NODE:
			weight = g_locate_cfg->weight_indirect_in_node;
			break;
	}

	return weight;
}

static int set_flag(long status, int flag)
{
	int tmp = 1 << flag;

	return status | tmp;
}

static int check_flag(long status, int flag)
{
	int tmp;

	tmp = 1 << flag;
	long ret = status & tmp;
	return ret;
}

static int is_near_node(double node_lon, double node_lat, double lon, double lat)
{
	int dist = dist_p2p(node_lon, node_lat, lon, lat);

	if (dist < NODE_RANGE_DIST) {
		return 0;
	} else {
		return -1;
	}
}

static int is_in_node_range(topo_node_t *p_node, double lon, double lat, int dir)
{
	if ((is_near_node(p_node->L1, p_node->B1, lon, lat) == 0)
		|| (is_near_node(p_node->L1, p_node->B1, lon, lat) == 0)) {
		return 0;
	} else {
		return -1;
	}
}

static void add_weight(double *p_weight, int flag)
{
	*p_weight += match_weight(flag);
}

static int calc_weight(double *p_weight, road_info_t *p_last, road_info_t *p_road, gps_point_t *p_pt, long *p_status)
{
	topo_node_t     node;
	int             flag = MATCH_FLAG_NO;

	// 连接类型加成MAX_FLAG_SIZE
	if (p_last->p_sg == p_road->p_sg) {	// 相同道路
		flag = MATCH_FLAG_LAST_SAM;
	} else {
		int ret = map_topo_isconnect(p_last->p_sg->rrid, p_last->p_sg->sgid, p_road->p_sg->rrid, p_road->p_sg->sgid, TRACK_BACK_DEEP, &node);
		flag = MATCH_FLAG_NO;

		switch (ret)
		{
			case TOPO_CONN_TYPE_NO:
				flag = MATCH_FLAG_NO;
				break;

			case TOPO_CONN_TYPE_DIRECT:
				flag = MATCH_FLAG_CONN_DIRECT;
				break;

			case TOPO_CONN_TYPE_INDIRECT:
				flag = MATCH_FLAG_CONN_INDIRECT;

				if (0 == is_near_node(p_road->p_sg->start_lon, p_road->p_sg->start_lat, p_pt->lon, p_pt->lat)) {
					add_weight(p_weight, MATCH_FLAG_INDIRECT_IN_NODE);
					p_road->status = set_flag(p_road->status, MATCH_FLAG_INDIRECT_IN_NODE);
				}

				break;

			case TOPO_CONN_TYPE_BYROAD:
				flag = MATCH_FLAG_BYROAD;

				// 辅路加成
				if (is_in_node_range(&node, p_pt->lon, p_pt->lat, p_pt->dir) == 0) {
					add_weight(p_weight, MATCH_FLAG_BYROAD);
					p_road->status = set_flag(p_road->status, MATCH_FLAG_BYROAD);
				}

				break;
		}
	}

	p_road->status = set_flag(p_road->status, flag);
	add_weight(p_weight, flag);

	return 0;
}

int get_nearst_road(match_road_t *p_match)
{
	int     dist = 100;
	int     idx = -1;
	int     i;

	for (i = 0; i < p_match->match_size; i++) {
		if (p_match->roads[i].dist < dist) {
			dist = p_match->roads[i].dist;
			idx = i;
		}
	}

	if (dist == 100) {
		return -1;
	} else {
		return idx;
	}
}

static void get_status_name(long status, char *buff, int size)
{
	int i;

	for (i = 0; i < MAX_FLAG_SIZE; i++) {
		if (check_flag(status, i) != 0) {
			char tmp[100] = { 0 };
			sprintf(tmp, "%s:%f,", match_status_name[i], match_weight(i));
			strcat(buff, tmp);
		}
	}
}

static void show_road_info(const char *message, road_info_t *p_road)
{
	char buff[1024];

	memset(buff, 0, 1024);
	get_status_name(p_road->status, buff, 1024);
	x_printf(I, "IMPORT:%s, %ld%03d, %s weight:%f, %s", message, p_road->p_sg->rrid, p_road->p_sg->sgid,
		p_road->p_sg_name, p_road->weight, buff);
}

static int show_user_road(user_info_t *p_user)
{
	char    buff[1024] = { 0 };
	int     i;

	for (i = 0; i < p_user->count; i++) {
		if (p_user->roads[i].p_sg_name) {
			strcat(buff, p_user->roads[i].p_sg_name);
		} else {
			strcat(buff, "nil");
		}

		strcat(buff, ", ");
	}

	x_printf(I, "IMPORT:%s last_road%s", p_user->IMEI, buff);

	return 0;
}

int match_roads(match_road_t *p_match, char *p_imei)
{
	user_info_t *p_user = user_info_get(p_imei);

	if (!p_user) {
		return -1;
	}

	int ret_idx = -1;

	// 1先判断连接类型
	// 2当道路无连接时在回溯上条道路
	// 3距离加成在道路有连接的情况下再处理
	// 4若回溯道路也无连接，则选取最近的一条道路
	road_info_t max_road;
	max_road.weight = 0.0;
	max_road.status = set_flag(max_road.status, MATCH_FLAG_NO);

	int idx;

	for (idx = 0; idx < p_user->count; idx++) {
		int i;

		for (i = 0; i < p_match->match_size; i++) {
			double          new_weight = 0;
			road_info_t     *p_road = &(p_match->roads[i]);
			calc_weight(&new_weight, &(p_user->roads[idx]), p_road, &(p_match->cur_pt), &(p_road->status));
			p_road->weight = new_weight;

			if (p_road->weight > max_road.weight) {
				max_road = *p_road;
				ret_idx = i;
			}

			show_road_info("match: ", &(p_match->roads[i]));
		}

		// 若有道路连接则不回溯道路
		if (check_flag(max_road.status, MATCH_FLAG_NO) == 0) {
			break;
		}
	}

	// 最近距离加成
	int dist_ret = get_nearst_road(p_match);

	if (dist_ret < 0) {
		return -1;
	} else {
		p_match->roads[dist_ret].status = set_flag(p_match->roads[dist_ret].status, MATCH_FLAG_NEAEST);
		add_weight(&(p_match->roads[dist_ret].weight), MATCH_FLAG_NEAEST);
	}

	if (ret_idx == -1) {
		max_road = p_match->roads[dist_ret];
		ret_idx = dist_ret;
	} else if (dist_ret != ret_idx) {
		if ((max_road.dist - p_match->roads[dist_ret].dist) <= WEIGHT_DIST_LIMIT) {
			max_road.status = set_flag(max_road.status, MATCH_FLAG_NEAEST);
			add_weight(&(max_road.weight), MATCH_FLAG_NEAEST);
		}

		if ((dist_ret != ret_idx) && (p_match->roads[dist_ret].weight > max_road.weight)) {
			max_road = p_match->roads[dist_ret];
			ret_idx = dist_ret;
		}
	}

	// 最小夹角加成

	show_user_road(p_user);
	show_road_info("final: ", &max_road);

	int ret = check_flag(max_road.status, MATCH_FLAG_LAST_SAM);

	if (ret == 0) {
		user_info_add(p_user, &max_road);
	}

	return ret_idx;
}

