#ifndef _KV_PRIV_H_
#define _KV_PRIV_H_

#include "libkv.h"

#ifdef __cplusplus
extern "C" {
#endif

kv_answer_t *kv_ask_proto(kv_handler_t *handler, const char *proto, unsigned int protolen);

#ifdef __cplusplus
}
#endif
#endif

