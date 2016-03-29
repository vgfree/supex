/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    loadfile_tomem.h
 * @detail  use libkv parse file to mem .
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#ifndef __LOADFILE_TOMEM_
#define __LOADFILE_TOMEM_

#include "rr_cfg.h"
#include "utils.h"
#include "redis_kv.h"
#include "libkv.h"
#include "gv_common.h"

FILE *load_file(const char *file);

int file_tokv(FILE *fp, kv_handler_t *handler);

// int bufHandle(char *buf, kv_handler_t *handler, char Alignment, int len);
int bufHandle(char *buf, kv_handler_t *handler, char Alignment);

void set_cmd(kv_handler_t *handler, const char *key, const char *code);
#endif

