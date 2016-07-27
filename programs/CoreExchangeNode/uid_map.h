#ifndef _UID_MAP_H_
#define _UID_MAP_H_

#include "stdint.h"
#include "libkv.h"

void init_uid_map();

int find_fd(char *uid);

int insert_fd(char *uid, int fd);

int find_uid(char *uid, int *size, int fd);

int remove_fd(char *uid);

int remove_uid(int fd);

void destroy_uid_map();

#endif

