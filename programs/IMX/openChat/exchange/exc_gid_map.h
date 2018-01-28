#pragma once

#include <stdint.h>

void exc_gidmap_init(void);

void exc_gidmap_free(void);

typedef void (*GID_FOR_EACH_CID)(char cid[MAX_CID_SIZE], size_t idx, void *usr);

int exc_gidmap_get_cid(char gid[MAX_GID_SIZE], GID_FOR_EACH_CID fcb, void *usr);

int exc_gidmap_add_cid(char gid[MAX_GID_SIZE], char cid[MAX_CID_SIZE]);

int exc_gidmap_rem_cid(char gid[MAX_GID_SIZE], char cid[MAX_CID_SIZE]);

