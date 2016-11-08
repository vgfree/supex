#include "traffic_info.h"
#include "rr_cfg.h"
#include "utils.h"
#include "async_tasks/async_api.h"
#include "pools/xpool.h"
#include "pool_api/conn_xpool_api.h"
#include "redis_api/redis_status.h"
#include "redis_parse.h"

#include <time.h>
 
#define ASYNC_LUAKV_THRESHOLD   1
 
extern struct rr_cfg_file g_rr_cfg_file;

static int add_redis_task(char command[], struct rr_link *link, struct ev_loop *loop)
{       
        struct xpool            *cpool = conn_xpool_find(link->host, link->port);
        struct async_api        *api = async_api_initial(loop, 1, true, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL);
        
        // x_printf(D,"import_to_redis: %s\n",command);
        
        if (api && cpool) {
                /*data*/
                char    *proto;
                int     ok = cmd_to_proto(&proto, command);
        
                if (ok == REDIS_ERR) {
                        if(proto)
                                free(proto);
 
                        async_api_distory(api);
                        return -1;
                }
        
                /*send*/
                struct command_node *cmd = async_api_command(api, PROTO_TYPE_REDIS, cpool, proto, strlen(proto), NULL, NULL);
                free(proto);
        
                if (cmd == NULL) {
                        async_api_distory(api);
                        return -1;
                }
        
                async_api_startup(api);
                return 0;
        }
        
        return -1;
}

int single_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop)
{                              
        int     old_rr_id = kv_roadID->old_roadID / 1000;
        int     old_sg_id = kv_roadID->old_roadID % 1000;
        int     save_time = g_rr_cfg_file.save_time;
        
        char    section[204] = "";
        single_road_section(section, kv_roadID);
                               
        /*添加单条道路路况*/   
        char redis_buff[BUFF_USE_LEN] = { 0 };
        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "SET %ld:roadSpeedInfo %d@%d@%ld@%ld@%s",
                        kv_roadID->old_roadID, kv_roadID->max_speed, kv_roadID->avg_speed,
                        kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
        x_printf(D, "redis command: %s", redis_buff);
                               
        add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), loop);
        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "hset %d:cityinfo %ld %ld@%d:%d@%s",
                        kv_roadID->citycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
        x_printf(D, "redis command: %s", redis_buff);
        add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "hset %d:countyinfo %ld %ld@%d:%d@%s",
                        kv_roadID->countycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
        x_printf(D, "redis command: %s", redis_buff);
        add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);
                           
        return 0;          
}       

int subsec_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop)
{                              
        int     old_rr_id = kv_roadID->old_roadID / 1000;
        int     old_sg_id = kv_roadID->old_roadID % 1000;
        int     save_time = g_rr_cfg_file.save_time;
        char    section[BUFF_SEC_LEN] = "";
        show_road_section(section, kv_roadID);
                               
        //添加单条道路路况   
        char redis_buff[BUFF_USE_LEN];
        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "SET %ld:roadSpeedInfo %s@%d@%d@%ld@%ld@%s",
                        kv_roadID->old_roadID, section, kv_roadID->max_speed, kv_roadID->avg_speed,
                        kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
        x_printf(D, "subsection redis command: %s", redis_buff);
                               
        add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), loop);

        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "hset %d:cityinfo %ld %ld@%d:%d@%s",
                        kv_roadID->citycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
        x_printf(D, "redis command: %s", redis_buff);
        add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);
        memset(redis_buff, 0, BUFF_USE_LEN);
        sprintf(redis_buff,    
                        "hset %d:countyinfo %ld %ld@%d:%d@%s",
                        kv_roadID->countycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
        x_printf(D, "redis command: %s", redis_buff);
        add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);
                           
        return 0;          
}
