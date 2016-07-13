/**
 * copyright (c) 2015
 * All Rights Reserved
 *
 * @file    loadfile_tomem.c
 * @detail  use libkv parse file to mem .
 *
 * @author  shumenghui
 * @date    2015-10-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "redis_kv.h"
#include "libkv.h"
#include "loadfile_tomem.h"

#define DATA_SIZE       (16 * 1024 * 512)
#define keyCursor       10
#define CtCursor        6
#define Cursor          16

extern struct rr_cfg_file g_rr_cfg_file;

FILE *load_file(const char *file)
{
	FILE *fp;

	fp = fopen(file, "r");

	if (!fp) {
		x_printf(E, "open error!. \n");
		return NULL;
	}

	return fp;
}

/*int file_tokv(FILE *fp, kv_handler_t *handler)
 *   {
 *        if (!fp) {
 *                x_printf(E, "open error!. \n");
 *                return -1;
 *        }
 *
 *        char *buf = (char*)malloc(DATA_SIZE + 1);
 *        int num = fread(buf, 1, DATA_SIZE, fp);
 *        while(num >= 0) {
 *
 *                buf[num] = '\0';
 *                bufHandle(buf, handler, '#', num);
 *                //memset(buf, 0, num);
 *                if(feof(fp)) break;
 *                num = fread(buf, 1, DATA_SIZE, fp);
 *        }
 *        free(buf);
 *        fclose(fp);
 *        return 0;
 *   }*/
int file_tokv(FILE *fp, kv_handler_t *handler)
{
	if (!fp) {
		x_printf(E, "open error!. \n");
		return GV_ERR;
	}

	char *buf = (char *)malloc(DATA_SIZE + 1);

	if (!buf) {
		x_printf(E, "malloc error.! \n");
		return GV_ERR;
	}

	while (fread(buf, DATA_SIZE, 1, fp) >= 0) {
		buf[DATA_SIZE] = '\0';
		bufHandle(buf, handler, '#');
		memset(buf, 0, DATA_SIZE);

		if (feof(fp)) {
			break;
		}
	}

	free(buf);
	fclose(fp);
	return GV_OK;
}

static int cmp(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

static int binary_search(char *code)
{
	int     citycode = atoi(code);
	int     left = 0;
	int     right = g_rr_cfg_file.city_size - 1;
	int     mid;

	while (left <= right && left >= 0 && right < g_rr_cfg_file.city_size) {
		mid = (left + right) >> 1;

		if (g_rr_cfg_file.citycode[mid] == citycode) {
			x_printf(I, "binary searched cityCode:%d\n", citycode);
			return GV_OK;
		}

		if (g_rr_cfg_file.citycode[mid] < citycode) {
			left = mid + 1;
		} else if (g_rr_cfg_file.citycode[mid] > citycode) {
			right = mid - 1;
		}
	}

#ifdef _BJSH
	int tmp = citycode / 100;

	if ((tmp == 3101) || (tmp == 3102) || (tmp == 1101) || (tmp == 1102)) {
		x_printf(D, "binary searched cityCode:%d\n", citycode);
		return GV_OK;
	}
#endif
	return GV_ERR;
}

int bufHandle(char *buf, kv_handler_t *handler, char Alignment)
{
	if (!buf) {
		x_printf(E, "filebuf is null. !\n");
		return GV_ERR;
	}

	int     i = 0;
	int     len = DATA_SIZE;
	// int len = strlen(buf);

	while (i < len) {
		if (buf[i] == '\0') {
			break;
		}

		char    key[11] = "";
		char    code[7] = "";
		memcpy(key, buf + i, keyCursor);

		memcpy(code, buf + i + keyCursor, CtCursor);
                if(g_rr_cfg_file.city_size > 0) {
                        if(!binary_search(code)){
                                set_cmd(handler, key, code);
                        }
                }
                else {
                        set_cmd(handler, key, code);
                }
		i += Cursor;
	}

	x_printf(I, "i:%d\n", i);
	return GV_OK;
}

void set_cmd(kv_handler_t *handler, const char *key, const char *code)
{
	if (!key || !code) {
		x_printf(E, "key or code buf is NULL. !\n");
		return;
	}

	kv_answer_t     *ans;
	char            buf[22] = "";
	int             num = sprintf(buf, "set %.10s %.6s", key, code);
	libkv_cmd(handler, buf, num, &ans);
	kvanswer_free(ans);
}

/*int main()
 *   {
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        kv_handler_t *handler;
 *        kv_answer_t *ans;
 *        handler = kv_create(NULL);
 *        FILE *fp = load_file("Hkeys-ct");
 *        file_tokv(fp, handler);
 *
 *        libkv_cmd(handler,"get 10671&2208", strlen("get 10671&2208"), &ans);
 *        get_allhashvaule(ans);
 *        kvanswer_free(ans);
 *
 *        libkv_cmd(handler,"get 12075&3690", strlen("get 12075&3690"), &ans);
 *        get_allhashvaule(ans);
 *        kvanswer_free(ans);
 *
 *        libkv_cmd(handler,"get 8185&3181", strlen("get 8185&3181"), &ans);
 *        get_allhashvaule(ans);
 *        kvanswer_free(ans);
 *
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        kvhandler_destroy(handler);
 *        x_printf(E, "\nused_memory before:%d\n", kv_get_used_memory());
 *        return 0;
 *   }*/

