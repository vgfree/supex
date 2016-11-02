#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "libkv.h"
#include "mttpsvp_libkv.h"
#include "sniff_evcoro_lua_api.h"


#define CMD_BUF_SIZE 1024

void mttpsvp_libkv_init()
{
	kv_init();
	int idx = kv_load(NULL, NULL);
	assert(idx == 0);
}

void mttpsvp_libkv_destory()
{
	kv_destroy();
}

void mttpsvp_libkv_set(uint64_t cid, int sfd, const char *v)
{
	char            cmd[CMD_BUF_SIZE];
	int             cmdlen = sprintf(cmd, "set %" PRId64 ":%d %s", cid, sfd, v);
	kv_handler_t *handler = kv_spl(0, cmd, cmdlen);
	if (handler == NULL) {
		x_printf(E, "libkv kv_spl error: not enough memory for kv_answer_t\n");
		return;
	}
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}

int mttpsvp_libkv_check_handshake(uint64_t cid, int sfd, const char *v, size_t vlen)
{
	char            cmd[CMD_BUF_SIZE];
	int             cmdlen = sprintf(cmd, "get %" PRId64 ":%d", cid, sfd);
	kv_handler_t *handler = kv_spl(0, cmd, cmdlen);
	if (handler == NULL) {
		x_printf(E, "libkv kv_spl error: not enough memory for kv_answer_t\n");
		return -1;
	}
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return -1;
	}

	unsigned long len = answer_length(ans);

	if (len != 1) {
		x_printf(E, "libkv kv_spl error: unknown answer length(%" PRId64 ")\n", len);
		kv_handler_release(handler);
		return -1;
	}

	kv_answer_value_t *value = answer_head_value(ans);
	char *data = (char *)answer_value_look_addr(value);
	size_t size = answer_value_look_size(value);
	if (size != vlen) {
		x_printf(E, "libkv kv_answer_first_value error: unknown value length(%" PRId64 ")\n", size);
		kv_handler_release(handler);
		return -1;
	}

	if (strncmp(data, v, vlen) == 0) {
		kv_handler_release(handler);
		return 0;
	}

	kv_handler_release(handler);
	return -1;
}

void mttpsvp_libkv_del(uint64_t cid, int sfd)
{
	char            cmd[CMD_BUF_SIZE];
	int             cmdlen = sprintf(cmd, "del %" PRId64 ":%d", cid, sfd);
	kv_handler_t *handler = kv_spl(0, cmd, cmdlen);
	if (handler == NULL) {
		x_printf(E, "libkv kv_spl error: not enough memory for kv_answer_t\n");
		return;
	}
	kv_answer_t *ans = &handler->answer;

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		return;
	}

	kv_handler_release(handler);
}

