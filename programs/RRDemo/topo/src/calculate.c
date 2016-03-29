/*
 *  calculate.c
 *
 *  Created on: Feb 25, 2016
 *  Author: shu
 * */

#include <string.h>
#include <stdlib.h>

#include "calculate.h"
#include "cJSON.h"
#include "topo_com.h"
#include "binheap.h"
#include "fetch_road.h"
#include "smart_api.h"

extern PriorityQueue g_H; 
static cJSON *getjsonitem(cJSON *obj, const char *item);

int decode_data(const char *data, VEHICLE_OBJ *veh)
{
        if (!data) {
                printf("receive vehicle data is nil!\n");
                return 0;
        }

        cJSON *roadid = NULL;
        cJSON *speed = NULL;
        cJSON *imei = NULL;

        cJSON *obj = cJSON_Parse(data);

        if (!obj) {
                printf("failed to parse json data!\n");
                goto jsonerr;
        }

        roadid = getjsonitem(obj, "roadid");
        if (!roadid) {
                printf("get nodeid error!\n");
                goto jsonerr;
        }

        speed = getjsonitem(obj, "speed");
        if (!speed) {
                printf("get longitute error!\n");
                goto jsonerr;
        }

        imei = getjsonitem(obj, "imei");
        if (!imei) {
                printf("get latitude error!\n");
                goto jsonerr;
        }
        
        //printf(">%s %s %s\n", roadid->valuestring, speed->valuestring, imei->valuestring);
        veh->roadid = atoi(roadid->valuestring);
        veh->speed  = atoi(speed->valuestring);
        veh->imei  = atoll(imei->valuestring);

        cJSON_Delete(obj);
        return 1;

jsonerr:
        cJSON_Delete(obj);
        return 0;
}

cJSON *getjsonitem(cJSON *obj, const char *item)
{       
        cJSON *retitem = NULL;

        retitem = cJSON_GetObjectItem(obj, item);

        if (!retitem) {
                printf("Failed to decode %s .\n", item);

                return NULL;
        }

        return retitem;
}

int calculate_priority(VEHICLE_OBJ *veh)
{
        int len = get_length_of_road(veh->roadid);
        veh->priority = len/100/veh->speed;
        printf("priority %d\n", veh->priority);
        return 1;
}

int update_priority(VEHICLE_OBJ *veh)
{
        if (veh == NULL || veh->speed == 0) {
                printf("obj is NULL or speed is NULL\n");
                return -1;
        }

        if (veh->magic == 1) {
                uint64_t nextroad = get_nextrandomroad_of_road(veh->roadid);
                if (nextroad > 0) {
                        veh->roadid = nextroad;
                        if (calculate_priority(veh) > 0)
                                Insert(veh, g_H);
                }
        }
        else {
                veh->magic = 1;
                if (calculate_priority(veh) > 0) {
                        Insert(veh, g_H);
                        printf("@@ %d \n", veh->roadid);
                }
        }

        return 1;
}

void update_call()
{
        ElementType min;
        ElementType cur;
        min = FindMin(g_H);

        if (min->priority != MinData) {
                printf("min:%d\n", min->priority);
                if (min->priority - g_H->lazy == 0) {
                        Update(g_H, g_H->lazy);
                        while( g_H->count-- ) {
                                cur = DeleteMin(g_H);
                                update_priority(cur);
                        }

                        g_H->count = 0;
                        g_H->lazy  = 0;
                }
                else {
                        g_H->lazy++;//最后一次递归时第二个条件不满足，必会执行一遍
                        printf("lazy:%d\n", g_H->lazy);
                }
        }
}
