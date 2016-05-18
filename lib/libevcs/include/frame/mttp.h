#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define HEADLEN         6
#define MTP_VERSION     0x10
#define MTP_FORMAT      0x20

// typedef int (*GZIPDECOMPRESS_CB)(const char *src, int srcLen, char *dst);
// typedef int (*PARSEFROMSTRING_CB)(char *src, size_t size, void **gpslist);

typedef enum mttp_errno
{
	MP_ok = 0,
	MP_head,
	MP_parse,
	MP_undo,
	MP_re,
	MP_err
} MTTP_ERRNO;

struct mttp_status
{
	unsigned short  mttp_version    : 8;
	unsigned short  encryption      : 8;
	size_t          body_size;
	unsigned short  headlen;
	unsigned short  step;
	bool            over;
	MTTP_ERRNO      err;

	int             dosize;			// 已分析的长度
	char *const     *buff;			// 被解析数据起始地址指针
	int const       *size;			// 当前数据的总长度地址
	// void            *gps_list;                  //解析完成数据地址
	int             *current;		// 当前放弃数据长度地址
};

struct mttp_parse_info
{
	// GZIPDECOMPRESS_CB       ungzip;
	// PARSEFROMSTRING_CB      unserialize;
	struct mttp_status mt;		/**< MTTP数据分析器状态*/
};

void mttp_parse_init(struct mttp_parse_info *info, char *const *buff, int const *size, int *outsize);

bool mttp_parse(struct mttp_parse_info *parseinfo);

