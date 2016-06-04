#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "libkv.h"
#include "mttpsvp_libkv.h"
#include "sniff_evcoro_lua_api.h"

static kv_handler_t *handler;

#define CMD_BUF_SIZE 1024

void mttpsvp_libkv_init() {
	handler = kv_create(NULL);
  if (handler == NULL) {
		x_printf(E, "libkv create error\n");
		exit(EXIT_FAILURE);
  }
}

void mttpsvp_libkv_destory() {
  kv_destroy(handler);
}

void mttpsvp_libkv_set(uint64_t cid, int sfd, const char *v) {
  char cmd[CMD_BUF_SIZE];
  int cmdlen = sprintf(cmd, "set %"PRId64":%d %s", cid, sfd, v);
		x_printf(E, "cmd: %s\n", cmd);
	kv_answer_t *ans = kv_ask(handler, cmd, cmdlen);
  if (ans == NULL) {
		x_printf(E, "libkv kv_ask error: not enough memory for kv_answer_t\n");
    return;
  }

  if (ans->errnum) {
		x_printf(E, "libkv kv_ask error: %s\n", ans->err);
  }

	kv_answer_release(ans);
}

int mttpsvp_libkv_check_handshake(uint64_t cid, int sfd, const char *v, size_t vlen) {
  char cmd[CMD_BUF_SIZE];
  int cmdlen = sprintf(cmd, "get %"PRId64":%d", cid, sfd);
		x_printf(E, "cmd: %s\n", cmd);
	kv_answer_t *ans = kv_ask(handler, cmd, cmdlen);
  if (ans == NULL) {
		x_printf(E, "libkv kv_ask error: not enough memory for kv_answer_t\n");
    return -1;
  }

  if (ans->errnum) {
		x_printf(E, "libkv kv_ask error: %s\n", ans->err);
	  kv_answer_release(ans);
    return -1;
  }

  unsigned long len = kv_answer_length(ans);
  if (len != 1) {
		x_printf(E, "libkv kv_ask error: unknown answer length(%"PRId64")\n", len);
	  kv_answer_release(ans);
    return -1;
  }

	kv_answer_value_t *value = kv_answer_first_value(ans);
  if (value == NULL) {
	  kv_answer_release(ans);
		x_printf(E, "libkv kv_answer_first_value error: not enough memory for kv_answer_value_t\n");
    return -1;
  }

  if (value->ptrlen != vlen) {
		x_printf(E, "libkv kv_answer_first_value error: unknown value length(%"PRId64")\n", value->ptrlen);
	  kv_answer_release(ans);
    return -1;
  }

		x_printf(E, "value: %s\n", (char *)value->ptr);
  if (strncmp(value->ptr, v, vlen) == 0) {
	  kv_answer_release(ans);
    return 0;
  }

	kv_answer_release(ans);
  return -1;
}

void mttpsvp_libkv_del(uint64_t cid, int sfd) {
  char cmd[CMD_BUF_SIZE];
  int cmdlen = sprintf(cmd, "del %"PRId64":%d", cid, sfd);
	kv_answer_t *ans = kv_ask(handler, cmd, cmdlen);
  if (ans == NULL) {
		x_printf(E, "libkv kv_ask error: not enough memory for kv_answer_t\n");
    return;
  }

  if (ans->errnum) {
		x_printf(E, "libkv kv_ask error: %s\n", ans->err);
  }

	kv_answer_release(ans);
}
