/*
 * 版权声明：暂无
 * 文件名称：map_seg.h
 * 创建者   ：王张彦
 * 创建日期：2015/11/18
 * 文件描述：计算前方seg
 * 历史记录：
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "seg_cfg.h"

#define  ITERATOR_COUNT 20

typedef struct map_seg_info
{
	uint8_t         sgid_rt;		// 路等级
	uint8_t         sgid_count;		// SGID数量
	uint8_t         start_grade;		// 起始路口等级
	uint8_t         end_grade;		// 终止路口等级
	uint8_t         sgid_id;		// 路段奇偶编号
	uint8_t         sgid;			// SG编号
	uint8_t         next_sgid;		//下一路SGID
        uint32_t        countyCode;          // TODO
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
} map_seg_info;

typedef struct rrid_buf
{
	uint8_t         sgid_count;
	map_seg_info    *ptr_sgid;
} rrid_buf;

typedef struct map_seg_manager
{
	// uint64_t  destory_long;
	uint64_t        load_once;
	int64_t         index_long;
	int64_t         rrid_buf_long;
	int64_t         name_index_long;
	int64_t         name_buf_long;
	// uint64_t  min_index;
	// uint64_t  max_index;
	rrid_buf        **ptr_index;
	char            ***p_name;
} map_seg_manager;

typedef struct  back_seg
{
	map_seg_info    *ptr_seg;
	char            *ptr_name;
	char            *ptr_start_name;
	char            *ptr_end_name;
} back_seg;

typedef struct  map_iterator_element
{
	uint64_t        rrid;
	uint8_t         sgid;
	uint8_t         sgid_count;
	uint8_t         next_sgid;
	uint64_t        next_rrid;
	uint64_t        surplus_end_grade_one_number;
	uint8_t         iterator_count;
} map_iterator_element;

uint32_t seg_file_load(char *file_name);

int map_seg_query(unsigned int rrid_id, unsigned int sgid, back_seg *buf_seg);

int map_seg_destory();

map_iterator_element *map_seg_iterator_init(unsigned int rrid_id, unsigned int sgid, unsigned int end_grade_one_number);

back_seg *map_seg_iterator_next(map_iterator_element *back_seg_element, back_seg *out);

int map_seg_iterator_destory(map_iterator_element *back_seg_element);

