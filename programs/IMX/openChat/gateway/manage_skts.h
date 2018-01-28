#pragma once
#include <pthread.h>

#define MAX_MANAGE_SKTS 10240

struct manage_skts
{
	int                     skts[MAX_MANAGE_SKTS];
	int                     used;
	pthread_rwlock_t        rwlock;
};

void manage_skts_init(struct manage_skts *mng);

void manage_skts_free(struct manage_skts *mng);

void manage_skts_add(struct manage_skts *mng, int sockfd);

void manage_skts_del(struct manage_skts *mng, int sockfd);

#ifndef MANAGE_TRAVEL_FOR_DELETE_FCB
typedef bool (*MANAGE_TRAVEL_FOR_DELETE_FCB)(int sockfd, int idx, void *usr);
#endif
#ifndef MANAGE_TRAVEL_FOR_LOOKUP_FCB
typedef bool (*MANAGE_TRAVEL_FOR_LOOKUP_FCB)(int sockfd, int idx, void *usr);
#endif

int manage_skts_travel(struct manage_skts *mng, MANAGE_TRAVEL_FOR_LOOKUP_FCB lfcb, MANAGE_TRAVEL_FOR_DELETE_FCB dfcb, void *usr);

