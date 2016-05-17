#ifndef _GID_MAP_H_
#define _GID_MAP_H_

#include "stdint.h"
#include "libkv.h"

#define GROUP_SIZE 1000


void init_gid_map();

int find_fd_list(char *gid, int fd_list[]);

int insert_fd_list(char *gid, int fd_list[], int size);

int insert_gid_list(int fd, char *gid);

int find_gid_list(int fd, char *gid[], int *size);

int remove_gid_list(int fd, char *gid[], int size);

int remove_fd_list(char *gid, int fd_list[], int size);

void destroy_gid_map();

#endif
