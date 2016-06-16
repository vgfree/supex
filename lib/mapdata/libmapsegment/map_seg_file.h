/*
 * 版权声明：暂无
 * 文件名称：map_seg_file.h
 * 创建者   ：王张彦
 * 创建日期：2015/11/18
 * 文件描述：计算前方seg
 * 历史记录：
 */

#pragma once

#include "map_seg.h"
#include "seg_cfg.h"

typedef struct file_seg_info
{
	uint8_t         sgid_rt;		// 路等级
	uint8_t         sgid_count;		// SGID数量
	uint8_t         start_grade;		// 起始路口等级
	uint8_t         end_grade;		// 终止路口等级
	uint8_t         sgid_id;		// 路段奇偶编号
	uint8_t         sgid;			// SG编号
	uint8_t         next_sgid;		//下一路SGID
	uint32_t        countyCode;
	uint64_t        next_rrid;		//下一路段编号
	uint64_t        rrid;			// 路段编号
	uint64_t        name_id;		// 道路名称编号
	uint64_t        start_name;		// 起始路口名称
	uint64_t        end_name;		// 终止路口名称
	uint64_t        length;			// SGID长度
	double          start_lon;		// 起始路口经度
	double          start_lat;		// 起始路口纬度
	double          end_lon;		// 终止路口经度
	double          end_lat;		// 终止路口纬度
	char            sg_name[58];		// 道路名称
} file_seg_info;

typedef struct sgid_file_header
{
	uint64_t        max_index;
	uint64_t        min_index;

	uint64_t        index_long;
	uint64_t        rrid_buf_long;
	uint64_t        name_index_long;
	uint64_t        name_buf_long;

	uint64_t        load_once;
	file_seg_info   *p_seg_buf;
} sgid_file_header;

int map_seg_file(char *seg_line_file, map_seg_cfg *p_seg_cfg);

