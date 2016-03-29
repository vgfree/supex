#pragma once
#include <stdint.h>
typedef struct topo_cfg_mysql_t{
        char            host[32];
        char            username[32];
        char            passwd[32];
        char            database[32];
        char            table[32];
        char            map_topo_file[128];
        uint16_t  port;
        uint16_t  load_once;
        uint32_t   max_id;
}topo_cfg_mysql_t;

typedef struct topo_cfg_file_t{
        char topo_file[128];
        uint16_t  rrid_index_len;
}topo_cfg_file_t;

int topo_cfg_init_mysql(char *p_name, topo_cfg_mysql_t *p_cfg);

int topo_cfg_init_file(char *p_name, topo_cfg_file_t *p_cfg);
