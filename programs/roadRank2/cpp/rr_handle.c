#include "rr_handle.h"
#include "gps_info.h"
#include "traffic_model.h"
#include "road_info.h"
#include "rr_cfg.h"

extern struct rr_cfg_file       g_rr_cfg_file;
extern TRAFFIC_MODEL            g_traffic_model_cfg;

static void get_road_info_callback(struct ev_loop *p_loop, gps_info_t *p_gps, road_info_t *p_road)
{
	calculate_start(p_loop, p_road, p_gps, &g_traffic_model_cfg);
}

int rr_task_handle(struct ev_loop *p_loop, char *p_data)
{
	int             ret;
	gps_info_t      gps_info = { 0 };

	ret = gps_decode(p_loop, p_data, &gps_info, g_rr_cfg_file.forward_server);

	if (ret < 0) {
		return ret;
	}

	pmr_info_t *pmr_info = (pmr_info_t *)calloc(1, sizeof(pmr_info_t));
	pmr_info->pmr_link = g_rr_cfg_file.pmr_server;
	pmr_info->p_loop = p_loop;
	pmr_info->p_gps = gps_info;
	pmr_info->pmr_callback = get_road_info_callback;

	ret = get_road_info_v2(pmr_info);

	if ((ret < 0) && pmr_info) {
		x_printf(E, "PMR call error!\n");
		free(pmr_info);
	}

	return ret;
}

