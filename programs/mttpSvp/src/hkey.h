#ifndef _HKEY_H_
#define _HKEY_H_

#include "stdint.h"
#include "libkv.h"

void hkey_init();

void hkey_insert_fd(char *key, int fd);

int hkey_get_fd(char *key);

void hkey_del_fd(char *key);

char *hkey_find_key(int fd);

void hkey_del_fd_key(int fd);

void hkey_insert_value(char *key, char *value);

char *hkey_get_value(char *key);

void hkey_del_value(char *key);

void hkey_insert_offset(char *key, int offset);

int hkey_get_offset(char *key);

void hkey_del_offset(char *key);

void hkey_destroy();
#endif
