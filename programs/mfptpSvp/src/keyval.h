#pragma once

#include "stdint.h"
#include "libkv.h"


void keyval_init(void);

void keyval_destroy(void);


void key_set_val(char *key, char *value, int size);

char *key_get_val(char *key, int *size);

void key_del_val(char *key);


void sfd_set_key(int sfd, char *key);

char *sfd_get_key(int sfd);

void sfd_del_key(int sfd);
