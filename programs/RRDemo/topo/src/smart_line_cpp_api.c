#include <string.h>
#include <stdlib.h>

#include "smart_line_cpp_api.h"
#include "cJSON.h"
#include "topo_com.h"
#include "binheap.h"
#include "fetch_road.h"
#include "calculate.h"
#include "smart_api.h"
#include "utils.h"

extern PriorityQueue g_H; 

int insert_call(void *user, void *task)
{
	struct smart_task_node  *p_task = (struct smart_task_node *)task;
	struct data_node        *p_node = get_pool_addr(p_task->sfd);

	char                    *p_buf = p_node->recv.buf_addr;
	struct http_status      *p_rst = &p_node->http_info.hs;

        //printf("%s\n", p_buf);

        VEHICLE_OBJ *veh = ( VEHICLE_OBJ* )malloc( sizeof(VEHICLE_OBJ) );//FIXME 内存未释放
        memset(veh, 0, sizeof(veh));

        int ret = decode_data((const char *)(p_node->recv.buf_addr + p_rst->body_offset), veh);
        if (ret > 0) {
                update_priority(veh);
        }
        
	return 0;
}

