#pragma once
#include <stdint.h>

void exc_cidmap_init(void);

void exc_cidmap_free(void);

int exc_cidmap_set_uid(char cid[MAX_CID_SIZE], char uid[MAX_UID_SIZE]);

int exc_cidmap_get_uid(char cid[MAX_CID_SIZE], char uid[MAX_UID_SIZE]);

int exc_cidmap_del_uid(char cid[MAX_CID_SIZE]);

typedef void (*CID_FOR_EACH_GID)(char gid[MAX_GID_SIZE], size_t idx, void *usr);

int exc_cidmap_get_gid(char cid[MAX_CID_SIZE], CID_FOR_EACH_GID fcb, void *usr);

int exc_cidmap_add_gid(char cid[MAX_CID_SIZE], char gid[MAX_GID_SIZE]);

int exc_cidmap_rem_gid(char cid[MAX_CID_SIZE], char gid[MAX_GID_SIZE]);
