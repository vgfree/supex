#pragma once

void control_sockfd_manage_init(void);

void control_sockfd_manage_free(void);

void control_sockfd_manage_add(int sockfd);

void control_sockfd_manage_del(int sockfd);


#ifndef MANAGE_TRAVEL_FOR_DELETE_FCB
typedef bool (*MANAGE_TRAVEL_FOR_DELETE_FCB)(int sockfd, int idx, void *usr);
#endif
#ifndef MANAGE_TRAVEL_FOR_LOOKUP_FCB
typedef void (*MANAGE_TRAVEL_FOR_LOOKUP_FCB)(int sockfd, int idx, void *usr);
#endif

int control_sockfd_manage_travel(MANAGE_TRAVEL_FOR_LOOKUP_FCB lfcb, MANAGE_TRAVEL_FOR_DELETE_FCB dfcb, void *usr);
