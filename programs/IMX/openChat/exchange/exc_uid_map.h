#pragma once
#include <stdint.h>

void exc_uidmap_init(void);

void exc_uidmap_free(void);

int exc_uidmap_get_cid(char uid[MAX_UID_SIZE], char cid[MAX_CID_SIZE]);

int exc_uidmap_set_cid(char uid[MAX_UID_SIZE], char cid[MAX_CID_SIZE]);

int exc_uidmap_del_cid(char uid[MAX_UID_SIZE]);
