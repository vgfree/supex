#ifndef _UID_MAP_H_
#define _UID_MAP_H_

#include "stdint.h"
#include "libkv.h"


void init_uid_map();

int find_fd(char *uid);

int insert_fd(char *uid, int fd);

int remove_fd(char *uid);

void destroy_uid_map();

#endif
