/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    redis_kv.c
 * @detail  interface for libkv.
 *
 * @author  shumenghui
 * @date    2015-10-26
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libkv.h"
#include "redis_kv.h"

//
int libkv_cmd(kv_handler_t *hid, const char *cmd, unsigned int cmdlen, kv_answer_t **ans)
{
	if (!cmd || (cmdlen == 0)) {
		x_printf(E, "Invalid command, try again! .\n");
		return -1;
	}

	kv_handler_t *handler = kv_spl(hid, cmd, cmdlen);
	*ans = &handler->answer;
	
	if (ERR_NONE != (*ans)->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", (*ans)->errnum, error_getinfo((*ans)->errnum));
                kv_handler_release(handler);
		return -1;
        }

	//kv_handler_release(handler);

	return 0;
}

void kvanswer_free(kv_answer_t *ans)
{
	kv_handler_t *handler = NULL;//TODO : offset by ans
	kv_handler_release(handler);
}

void kvhandler_destroy(void)
{
	kv_destroy();
}

void get_allhashvaule(kv_answer_t *ans)
{
	unsigned long len = kv_answer_length(ans);

	if (len == 1) {
		int                     i;
		kv_answer_value_t       *value;

		x_printf(E, "Single value:\n");
		x_printf(E, "\tkv value:");
		value = kv_answer_first_value(ans);

		for (i = 0; i < value->ptrlen; i++) {
			x_printf(E, "%c", ((char *)value->ptr)[i]);
		}

		x_printf(E, "\n");
	} else {
		int                     i;
		kv_answer_value_t       *value;
		kv_answer_iter_t        *iter;

		x_printf(E, "Multi value:\n");
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);
		x_printf(E, "\tkv values:");

		while ((value = kv_answer_next(iter)) != NULL) {
			for (i = 0; i < value->ptrlen; i++) {
				x_printf(E, "%c", ((char *)value->ptr)[i]);
			}

			x_printf(E, "  ");
		}

		x_printf(E, "\n");

		kv_answer_release_iter(iter);
	}
}

/*
 *   int main(int argc, char **argv)
 *   {
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        kv_handler_t *handler;
 *        handler = kv_create(NULL);
 *        kv_answer_t *ans;
 *
 *        libkv_cmd(handler,"hset myhash field1 shushu", strlen("hset myhash field1 shushu"), &ans);
 *        kvanswer_free(ans);
 *        libkv_cmd(handler,"hset myhash field2 huhu", strlen("hset myhash field2 huhu"), &ans);
 *        kvanswer_free(ans);
 *        libkv_cmd(handler,"hget myhash field1", strlen("hget myhash field1"), &ans);
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        get_allhashvaule(ans);
 *        kvanswer_free(ans);
 *        libkv_cmd(handler,"hget myhash field2", strlen("hget myhash field2"), &ans);
 *        get_allhashvaule(ans);
 *        kvanswer_free(ans);
 *
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        kvhandler_destroy(handler);
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        return 0;
 *   }
 */

