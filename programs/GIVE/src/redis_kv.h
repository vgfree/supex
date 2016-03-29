/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    redis_kv.h
 * @detail  libkv interface
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#ifndef __REDIS_TOMEM__
#define __REDIS_TOMEM__

#include "utils.h"
#include "libkv.h"

int libkv_cmd(kv_handler_t *handler, const char *cmd, unsigned int cmdlen, kv_answer_t **ans);

void kvanswer_free(kv_answer_t *ans);

void kvhandler_destroy(kv_handler_t *handler);

void get_allhashvaule(kv_answer_t *ans);
#endif

