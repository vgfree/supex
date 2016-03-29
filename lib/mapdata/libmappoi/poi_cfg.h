#pragma once

typedef struct mappoi_cfg_mysql
{
	unsigned short  port;
	unsigned int    load_once;
	unsigned int    max_sg_id;
	unsigned int    max_poi_id;
	char            host[128];
	char            username[128];
	char            passwd[128];
	char            database[128];
	char            sg_table[128];
	char            poi_table[128];
	char            file_name_poi[128];
	char            file_name_sg[128];
} mappoi_cfg_mysql;

typedef struct mappoi_cfg_file
{
	char    file_name_poi[128];
	char    file_name_sg[128];
} mappoi_cfg_file;

int mappoi_load_cfg_mysql(mappoi_cfg_mysql *cfg, char *cfg_file_name);

int mappoi_load_cfg_data(mappoi_cfg_file *cfg, char *cfg_file_name);

