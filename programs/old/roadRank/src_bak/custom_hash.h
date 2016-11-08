#ifndef __CUSTOM_HASH__
#define __CUSTOM_HASH__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
unsigned int MurmurHash2(const void *key, int len, unsigned int seed);

int custom_hash_dist(char *_account, int _srv_num, int _suffix);
#endif

