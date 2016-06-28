#include "match_road.h"
#include "redis_parse.h"
#include "calculate.h"
#include "redis_api/redis_status.h"
#include "evcoro_async_tasks.h"
#include "spx_evcs.h"

#define ASYNC_LIBEV_THRESHOLD 10

extern struct rr_cfg_file g_rr_cfg_file;

struct user_data  
{                 
        char                    *p_buf;
        struct redis_status     *status;
}; 

int match_road_v2(struct ev_loop *loop, CAL_INFO *cal_info)
{
        int     len = 0;
        char    *proto = NULL;
	char    *host = g_rr_cfg_file.pmr_server.host;
	int     port = g_rr_cfg_file.pmr_server.port;

        struct user_data g_user_data;
        g_user_data.p_buf = NULL;
        g_user_data.status = NULL;
        struct supex_evcoro     *p_evcoro = supex_get_default();
        struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
        struct xpool            *cpool = conn_xpool_find(host, port);

        char cmd_buff[120] = "";
        snprintf(cmd_buff, 120, "hmget MLOCATE %s %lf %lf %d %d %d %ld", (cal_info->gps_info).IMEI, (cal_info->gps_info).longitude,(cal_info->gps_info).latitude, (cal_info->gps_info).direction, (cal_info->gps_info).altitude, (cal_info->gps_info).max_speed, (cal_info->gps_info).end_time);
        len = cmd_to_proto(&proto, cmd_buff);
        //len = cmd_to_proto(&proto, "hmget MLOCATE %s %lf %lf %d %d %d %ld", (cal_info->gps_info).IMEI, (cal_info->gps_info).longitude,(cal_info->gps_info).latitude, (cal_info->gps_info).direction, (cal_info->gps_info).altitude, (cal_info->gps_info).max_speed, (cal_info->gps_info).end_time);
        //len = cmd_to_proto(&proto, "hgetall %s", "web");
        //len = cmd_to_proto(&proto, "hmget MLOCATE 409601328282142roadRank 121.364435 31.224012 -1 32 0 1458711418");
        struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
        struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

        evtask_install(tasker);
        evtask_startup(p_scheduler);

        g_user_data.p_buf = cache_data_address(&command->cache);
        g_user_data.status = &command->parse.redis_info.rs;
        /*printf("redis data fields = %d\n", g_user_data.status->fields);
        int j = 0;
        for (j = 0; j < g_user_data.status->fields; j++) {
                printf("redis field[%d] = %s\n", j, g_user_data.p_buf + g_user_data.status->field[j].offset);
        } 
        */
        ROAD_INFO road_info = { 0 };
        
        if (command->err == ASYNC_OK || g_user_data.status->error == 0) {
                road_info.road_rootID = strtoll(g_user_data.p_buf + g_user_data.status->field[0].offset, NULL, 10);
                road_info.segmentID = strtol(g_user_data.p_buf + g_user_data.status->field[1].offset, NULL, 10);
                road_info.countycode = strtol(g_user_data.p_buf + g_user_data.status->field[3].offset, NULL, 10);
                road_info.rt = strtol(g_user_data.p_buf + g_user_data.status->field[4].offset, NULL, 10);

                road_info.citycode = (road_info.countycode / 100) * 100;
                road_info.new_roadID = road_info.road_rootID * 1000 + road_info.segmentID;

                x_printf(D, "road_info road_rootID %ld segmentID %d countycode %d rt %d citycode %d new_roadID %ld\n",
                        road_info.road_rootID, road_info.segmentID, road_info.countycode, road_info.rt,
                        road_info.citycode, road_info.new_roadID);

                struct ev_loop *loop = command->ev_hook->loop;

                cal_info->cal_callback(&(cal_info->gps_info), (void *)&road_info, loop);
                
        }
        evtask_distory(tasker);
        return 0;
}
