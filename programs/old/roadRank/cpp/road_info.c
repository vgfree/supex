#include "road_info.h"
#include <string.h>

#include "redis_parse.h"
#include "redis_api/redis_status.h"                                                                      
#include "evcoro_async_tasks.h"
#include "spx_evcs.h"

extern struct rr_cfg_file g_rr_cfg_file;

struct user_data  
{                 
        char                    *p_buf;
        struct redis_status     *status;
}; 

#if 0
//void PMR_callback(struct async_ctx *ac, void *reply, void *data)
void PMR_callback(struct async_obj *ac, void *reply, void *data)
{
        pmr_info_t * p_pmr = data;
        struct  redis_reply * p_reply = reply;

        if (p_reply && (p_reply->elements != 0)) {
                road_info_t road_info = {0};
                if (REDIS_REPLY_ARRAY == p_reply->type) {
                        road_info.rr_id = atoll(p_reply->element[0]->str);
                        road_info.sg_id = atoll(p_reply->element[1]->str);
                        road_info.county_code = atoll(p_reply->element[3]->str);
                        road_info.rt = atoi(p_reply->element[4]->str);
                        strncpy(road_info.road_name,p_reply->element[5]->str,strlen(p_reply->element[5]->str));
                        road_info.start_lon = strtod(p_reply->element[6]->str, NULL);
                        road_info.start_lat  = strtod(p_reply->element[7]->str, NULL);
                        road_info.end_lon = strtod(p_reply->element[8]->str, NULL);
                        road_info.end_lat = strtod(p_reply->element[9]->str, NULL);
                        road_info.len   = atoll(p_reply->element[10]->str);
                        road_info.city_code = (road_info.county_code / 100) * 100;
                        road_info.new_roadID = road_info.rr_id * 1000 + road_info.sg_id;
                        //printf("se:%d,sp_se:%d,road_info rr_id %ld,sg_id %d\n",p_pmr->sequence,p_pmr->speed_se,road_info.rr_id,road_info.sg_id);
                        x_printf(D,"pmr road_info: imei:%s gps_time:%ld gps_lon:%lf gps_lat:%lf gps_avg:%d roadname %s road_rootID %d segmentID %d countycode %d rt %d citycode %d new_roadID %ld len %d s_lon %lf s_lat %lf e_lon %lf e_lat %lf\n", p_pmr->p_gps.IMEI, p_pmr->p_gps.end_time, p_pmr->p_gps.longitude, p_pmr->p_gps.latitude, p_pmr->p_gps.avg_speed, road_info.road_name, road_info.rr_id, road_info.sg_id, road_info.county_code, road_info.rt, road_info.city_code, road_info.new_roadID, road_info.len, road_info.start_lon, road_info.start_lat, road_info.end_lon, road_info.end_lat);
                        struct ev_loop *loop = ac->settings.loop;
                        p_pmr->pmr_callback(loop,&(p_pmr->p_gps),&road_info);
                }
        }

        if( data )
                free(data);
}
static void pmr_abnormal_cb(const struct async_obj *obj, void *data)
{      
        struct command_node *p_work = obj->replies.work;

        if (p_work->privdata) {
                free(p_work->privdata);
                p_work->privdata = NULL;
        }
}      

static int forward_to_server(char *host, int port, const char *data, size_t size, struct ev_loop *loop, ASYNC_CALL_BACK fncb, void *with)
{
        struct cnt_pool         *cpool = NULL;
        struct async_ctx        *ac = NULL;

        ac = async_initial(loop, QUEUE_TYPE_FIFO, pmr_abnormal_cb, NULL, NULL, 1);
        if (ac) {
                void    *sfd = (void *)(intptr_t)-1;
                int     rc = pool_api_gain(&cpool, host, port, &sfd);

                if (rc) {
                        async_distory(ac);
                        return -1;
                }

                /*send*/
                async_command(ac, PROTO_TYPE_REDIS, (int)(intptr_t)sfd, cpool, fncb, with, data, size);
                async_startup(ac);

                return 0;
        }
        return -1;
}
int get_road_info(pmr_info_t *p_pmr)
{
        char * host = g_rr_cfg_file.pmr_server.host;
        int    port = g_rr_cfg_file.pmr_server.port;
        int    ok   = -1;
        char   buff[128] = {0};
        char * proto = NULL;

        x_printf(D, "before get road    IMEI:%s max_speed:%d min_speed:%d avg_speed:%d start_time:%ld end_time:%ld longitude:%lf latitude:%lf direction:%d tokencode:%s", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).max_speed, (p_pmr->p_gps).min_speed, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).start_time, (p_pmr->p_gps).end_time, (p_pmr->p_gps).longitude, (p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).tokenCode);

        sprintf(buff,"hmget MLOCATE %sroadRank %lf %lf %d %d %d %ld", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).longitude,(p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).altitude, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).end_time);
        x_printf(D, "redis command: %s", buff);
        cmd_to_proto(&proto, buff);

        if (proto) {
                ok = forward_to_server(host, port, proto, strlen(proto),p_pmr->p_loop, PMR_callback, p_pmr);
                free(proto);
        }
        return ok;
}
#endif
int get_road_info_v2(pmr_info_t *p_pmr)
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
        
        x_printf(D, "before get road    IMEI:%s max_speed:%d min_speed:%d avg_speed:%d start_time:%ld end_time:%ld longitude:%lf latitude:%lf direction:%d tokencode:%s", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).max_speed, (p_pmr->p_gps).min_speed, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).start_time, (p_pmr->p_gps).end_time, (p_pmr->p_gps).longitude, (p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).tokenCode);
         
        snprintf(cmd_buff, 120, "hmget MLOCATE %sroadRank %lf %lf %d %d %d %ld", (p_pmr->p_gps).IMEI, (p_pmr->p_gps).longitude,(p_pmr->p_gps).latitude, (p_pmr->p_gps).direction, (p_pmr->p_gps).altitude, (p_pmr->p_gps).avg_speed, (p_pmr->p_gps).end_time);
        x_printf(D, "redis command: %s", cmd_buff);

        len = cmd_to_proto(&proto, cmd_buff);
        struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
        struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

        evtask_install(tasker);
        evtask_startup(p_scheduler);

        g_user_data.p_buf = cache_data_address(&command->cache);
        g_user_data.status = &command->parse.redis_info.rs;

        if (g_user_data.status->fields != 11) {
                evtask_distory(tasker);
                return -1;
        }

        road_info_t road_info = {0};        
        if (command->err == ASYNC_OK || g_user_data.status->error == 0) {
                road_info.rr_id = strtoll(g_user_data.p_buf + g_user_data.status->field[0].offset, NULL, 10);
                road_info.sg_id = strtol(g_user_data.p_buf + g_user_data.status->field[1].offset, NULL, 10);
                road_info.county_code = strtol(g_user_data.p_buf + g_user_data.status->field[3].offset, NULL, 10);
                road_info.rt = strtol(g_user_data.p_buf + g_user_data.status->field[4].offset, NULL, 10);
                strncpy(road_info.road_name,g_user_data.p_buf + g_user_data.status->field[5].offset,g_user_data.status->field[5].len);
                road_info.start_lon = strtod(g_user_data.p_buf + g_user_data.status->field[6].offset, NULL);
                road_info.start_lat  = strtod(g_user_data.p_buf + g_user_data.status->field[7].offset, NULL);
                road_info.end_lon = strtod(g_user_data.p_buf + g_user_data.status->field[8].offset, NULL);
                road_info.end_lat = strtod(g_user_data.p_buf + g_user_data.status->field[9].offset, NULL);
                road_info.len   = strtoll(g_user_data.p_buf + g_user_data.status->field[10].offset, NULL, 10);
                road_info.city_code = (road_info.county_code / 100) * 100;
                road_info.new_roadID = road_info.rr_id * 1000 + road_info.sg_id;

                x_printf(D,"pmr road_info: imei:%s gps_time:%ld gps_lon:%lf gps_lat:%lf gps_avg:%d roadname %s road_rootID %d segmentID %d countycode %d rt %d citycode %d new_roadID %ld len %d s_lon %lf s_lat %lf e_lon %lf e_lat %lf\n", p_pmr->p_gps.IMEI, p_pmr->p_gps.end_time, p_pmr->p_gps.longitude, p_pmr->p_gps.latitude, p_pmr->p_gps.avg_speed, road_info.road_name, road_info.rr_id, road_info.sg_id, road_info.county_code, road_info.rt, road_info.city_code, road_info.new_roadID, road_info.len, road_info.start_lon, road_info.start_lat, road_info.end_lon, road_info.end_lat);

                struct ev_loop *loop = command->ev_hook->loop;
                p_pmr->pmr_callback(loop,&(p_pmr->p_gps),&road_info);
                
        }
        if(p_pmr)
                free(p_pmr);

        evtask_distory(tasker);
        return 0;
}
