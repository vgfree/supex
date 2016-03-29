/*
 * 版权声明：暂无
 * 文件名称：seg_cfg.h
 * 创建者   ：王张彦
 * 创建日期：2015/11/18
 * 文件描述：seg加载配置文件
 * 历史记录：
 */
#pragma once

typedef struct  map_seg_cfg
{
	char            host[32];
	char            username[32];
	char            passwd[32];
	char            database[32];
	char            table[32];
	char            map_seg_file[32];
	unsigned short  port;
	unsigned long   load_once;
	unsigned long   max_index;
	unsigned long   min_index;
	unsigned long   index_long;
	unsigned long   rrid_buf_long;
	unsigned long   name_index_long;
	unsigned long   name_buf_long;
} map_seg_cfg;

int seg_load_cfg_mysql(char *p_name, map_seg_cfg *p_seg_cfg);

