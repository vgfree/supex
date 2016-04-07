#include "fd_manager.h"

static fd_list list;

static int gateway_select()
{
	// TO DO
	    if (list.fd_head[MESSAGE_GATEWAY].size == 0) {
                return -1;
	    }
		else {
			return list.fd_head[MESSAGE_GATEWAY]->node.fd;
		}
}

int find_router_object(enum router_object obj)
{
	    int fd = -1;
	    switch (obj) {
		case CLIENT:
			break;
		case MESSAGE_GATEWAY:
            fd = gateway_select();
			break;
		case ROUTER_SERVER:
			break;
		default:
			//error handle.
	    }
		return fd;
}

int append_router_object(enum router_object obj, int fd)
{
	if (list.fd_head[MESSAGE_GATEWAY].size == 0) {
		// TO DO.
	}
	else {
		// TO DO.
	}
}
