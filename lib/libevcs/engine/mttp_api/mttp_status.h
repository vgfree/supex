#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define HEADLEN         7
#define MTP_VERSION     0x10
#define MTP_FORMAT      0x00

// typedef int (*GZIPDECOMPRESS_CB)(const char *src, int srcLen, char *dst);
// typedef int (*PARSEFROMSTRING_CB)(char *src, size_t size, void **gpslist);

typedef enum mttp_errno
{
	MP_ok = 0,
	MP_undo,
	MP_more,
	MP_erro
} MTTP_ERRNO;

struct mttp_status
{
	unsigned short  mttp_version    : 8;
	unsigned short  encryption      : 8;
	unsigned short  operation       : 8;

	unsigned short  head_len;
	unsigned int    body_len;

	MTTP_ERRNO      err;				/**< !=0 表示出错*/
	char *const     *data;				/**< 当前被分析数据首地址的指针*/
	unsigned const  *size;				/**< 当前被分析数据的长度地址*/
	bool            over;				/**< 当前分析是否结束*/
	unsigned        dosize;				/**< 当前已分析长度*/
	unsigned short  step;				/**< 当前分析进行了多少次*/
};

struct mttp_parse_info
{
	// GZIPDECOMPRESS_CB       ungzip;
	// PARSEFROMSTRING_CB      unserialize;
	struct mttp_status ms;		/**< MTTP数据分析器状态*/
};

void mttp_parse_init(struct mttp_parse_info *info, char *const *buff, unsigned const *size);

bool mttp_parse(struct mttp_parse_info *info);

void mttp_parse_free(struct mttp_parse_info *info);

