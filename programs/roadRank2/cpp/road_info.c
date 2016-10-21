#include "road_info.h"
#include <string.h>

#include "redis_parse.h"
#include "libevcs.h"

extern struct rr_cfg_file g_rr_cfg_file;

int get_road_info_v2(pmr_info_t *p_pmr)
{
	int     len = 0;
	char    *proto = NULL;
	char    *host = g_rr_cfg_file.pmr_server.host;
	int     port = g_rr_cfg_file.pmr_server.port;

	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find(host, port);

	char cmd_buff[120] = "";

	x_printf(D, "before get road    IMEI:%s max_speed:%d min_speed:%d avg_speed:%d start_time:%ld end_time:%ld longitude:%lf latitude:%lf direction:%d tokencode:%s", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).max_speed, (p_pmr->p_gps).min_speed, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).start_time, (p_pmr->p_gps).end_time, (p_pmr->p_gps).longitude, (p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).tokenCode);

	snprintf(cmd_buff, 120, "hmget MLOCATE %sroadRank %lf %lf %d %d %d %ld", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).longitude, (p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).altitude, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).end_time);
	x_printf(D, "redis command: %s", cmd_buff);

	len = cmd_to_proto(&proto, cmd_buff);
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);
	evtask_startup(p_scheduler);

	char                    *p_buf = cache_data_address(&command->cache);
	struct redis_status     *status = &command->parse.redis_info.rs;

	if (status->fields != 11) {
		evtask_distory(tasker);
		return -1;
	}

	road_info_t road_info = { 0 };

	if ((command->err == ASYNC_OK) || (status->error == 0)) {
		road_info.rr_id = strtoll(p_buf + status->field[0].offset, NULL, 10);
		road_info.sg_id = strtol(p_buf + status->field[1].offset, NULL, 10);
		road_info.county_code = strtol(p_buf + status->field[3].offset, NULL, 10);
		road_info.rt = strtol(p_buf + status->field[4].offset, NULL, 10);
		strncpy(road_info.road_name, p_buf + status->field[5].offset, status->field[5].len);
		road_info.start_lon = strtod(p_buf + status->field[6].offset, NULL);
		road_info.start_lat = strtod(p_buf + status->field[7].offset, NULL);
		road_info.end_lon = strtod(p_buf + status->field[8].offset, NULL);
		road_info.end_lat = strtod(p_buf + status->field[9].offset, NULL);
		road_info.len = strtoll(p_buf + status->field[10].offset, NULL, 10);
		road_info.city_code = (road_info.county_code / 100) * 100;
		road_info.new_roadID = road_info.rr_id * 1000 + road_info.sg_id;

		x_printf(D, "pmr road_info: imei:%s gps_time:%ld gps_lon:%lf gps_lat:%lf gps_avg:%d roadname %s road_rootID %d segmentID %d countycode %d rt %d citycode %d new_roadID %ld len %d s_lon %lf s_lat %lf e_lon %lf e_lat %lf\n", p_pmr->p_gps.IMEI, p_pmr->p_gps.end_time, p_pmr->p_gps.longitude, p_pmr->p_gps.latitude, p_pmr->p_gps.avg_speed, road_info.road_name, road_info.rr_id, road_info.sg_id, road_info.county_code, road_info.rt, road_info.city_code, road_info.new_roadID, road_info.len, road_info.start_lon, road_info.start_lat, road_info.end_lon, road_info.end_lat);

		struct ev_loop *loop = command->ev_hook->loop;
		p_pmr->pmr_callback(loop, &(p_pmr->p_gps), &road_info);
	}

	if (p_pmr) {
		free(p_pmr);
	}

	evtask_distory(tasker);
	return 0;
}

