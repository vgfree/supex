/*
 * 版权声明：暂无
 * 文件名称：map_line_file.h
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/30
 * 文件描述：load data from file
 * 历史记录：
 */
#pragma once

#include <stdint.h>
#include "pmr_cfg.h"
#include "libmini.h"

/*存储line的相关信息*/
typedef struct map_line_info
{
        uint8_t         sgid;
	uint8_t         tfid;
        int16_t         dir;
	uint32_t        rr_id;
        uint32_t        line_id;
	double          start_lon;
	double          start_lat;
	double          end_lon;
	double          end_lat;
} map_line_info;

/* 管理line_id的相关信息 */
typedef struct map_line_manager
{
	int64_t         max_line_id;
	int64_t         min_line_id;
	int64_t         destory_long;
	map_line_info   **ptr_arry;
} map_line_manager;

/*
 * 名    称：map_line_load_file
 * 功    能：从文件中加在line数据
 * 参    数：  file--加在文件名
 *                p_manage--管理line查询结构体指针
 * 返回值：0--加在成功
 *                其他--错误代码
 */
int32_t map_line_load_file(char *file, map_line_manager *p_manage, uint32_t load_once);

/*
 * 名    称：map_line_gen_file
 * 功    能：生成line数据，提供给map_line_load_file加载使用
 * 参    数：  file--加在文件名
 *                p_cfg--配置文件结构体指针
 * 返回值：0--加在成功
 *                其他--错误代码
 */
int32_t map_line_gen_file(char *file, map_line_load_cfg *p_cfg);

/*
 * 名    称：map_line_destory
 * 功    能：释放内存
 * 参    数：NULL
 * 返 回 值：0
 */
int32_t map_line_manager_destory(map_line_manager *p_manage);

