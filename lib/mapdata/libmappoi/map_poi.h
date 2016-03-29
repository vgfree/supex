#pragma once

// poi具体信息
typedef struct mappoi_poi
{
	short           poi_count;	// 当前sgid上有多少个poi
	short           ang;
	int             poi_type;
	int             county_code;
	int             city_code;
	unsigned int    poi_id;
	unsigned int    rr_id;
	unsigned int    sg_id;
	double          point_x;
	double          point_y;
} mappoi_poi;

// 一组poi信息
typedef struct mappoi_poi_buf
{
	mappoi_poi      *poi;		// 当前poi组信息
	int             poi_count;	// 当前poi组中poi个数
	int             poi_idx;
} mappoi_poi_buf;

// segment信息
typedef struct mappoi_sg
{
	unsigned int    rr_id;
	unsigned int    sg_id;
	unsigned int    next_rr_id;
	unsigned int    next_sg_id;
	unsigned int    sg_count;
	mappoi_poi_buf  poi_buf;// 当前poi组信息
} mappoi_sg;

typedef struct mappoi_sg_buf
{
	unsigned int    sg_count;
	mappoi_sg       *sg_buf;// segment组信息
} mappoi_sg_buf;

// roadroot信息
typedef struct mappoi_rr
{
	unsigned        rr_id;
	mappoi_sg_buf   sg;
} mappoi_rr;

// mappoi管理器
typedef struct mappoi_manager
{
	char            file_name_poi[128];
	char            file_name_sg[128];
	int             load_once;
	unsigned int    max_sg_id;
	unsigned int    max_poi_id;
	mappoi_rr       **rr_buf;	// roadroot组信息，每组有load_once个mappoi_rr结构体
	int             rr_index_count;	// rr_buf长度
} mappoi_manager;

// mappoi查询迭代器
typedef struct mappoi_iterator
{
	unsigned        rr_id;
	unsigned        sg_id;
	int             sg_count;
	int             sg_max;
} mappoi_iterator;

// 加载配置文件，读取配置文件中数据文件名
int mappoi_load_cfg(char *cfg_file_name);

// 开始加载数据到内存中
int mappoi_load();

mappoi_poi_buf *mappoi_query_poi(unsigned int rr_id, unsigned int sg_id);

// 初始化一个迭代器
mappoi_iterator *mappoi_iterator_init(unsigned rr_id, unsigned sg_id, int sg_max);

// 迭代器返回一个元素
// 0 su
// 1 er
int mappoi_iterator_next(mappoi_iterator *p_it, mappoi_poi_buf **poi_buf);

// 销毁迭代器
// void mappoi_iterator_destory(mappoi_poi_buf *iterator);
void mappoi_iterator_destory(mappoi_iterator *iterator);

