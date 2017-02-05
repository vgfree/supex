#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


struct sure_status
{
	size_t  length;

	char *const     *data;				/**< 当前被分析数据首地址的指针*/
	unsigned const  *size;				/**< 当前被分析数据的长度地址*/
	bool            over;				/**< 当前分析是否结束*/
	unsigned        dosize;				/**< 当前已分析长度*/
	unsigned short  step;				/**< 当前分析进行了多少次*/
};

struct sure_parse_info
{
	struct sure_status ss;		/**< 数据分析器状态*/
};

void sure_parse_init(struct sure_parse_info *info, char *const *buff, unsigned const *size, size_t todo);

bool sure_parse(struct sure_parse_info *info);

void sure_parse_free(struct sure_parse_info *info);

