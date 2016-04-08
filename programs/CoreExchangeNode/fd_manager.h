#ifndef _FD_MANAGER_H_
#define _FD_MANAGER_H_

#include "router.h"
#include "communication.h"

#define FD_MAX_CLASSIFICATION 3

/*    fd_head
 *    ---------         ---------         ---------
 *    | CLIENT| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------         ---------         ---------
 *    |GATEWAY| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------         ---------         ---------
 *      ...               ...               ...
 * */

struct fd_node;

struct fd_node {
	int fd;
	uint8_t status; // Descriptor fd connect status.
        struct fd_node *next;
};

struct fd_head {
        enum router_object obj;
	uint32_t size;
	struct fd_node *node;
};

struct fd_list {
        struct fd_head [FD_MAX_CLASSIFICATION];
};

int find_router_object(enum router_object obj);
int append_router_object(enum router_object obj, int fd);
#endif
