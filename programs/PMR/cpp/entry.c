#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "entry.h"
#include "map_pmr.h"
#include "map_seg.h"
#include "map_topo.h"
#include "json.h"
#include "libmini.h"


kv_cache                *g_kv_cache = NULL;
locate_cfg_t            *g_locate_cfg = NULL;

/*
 *   if ( ret == X_MALLOC_FAILED ){
 *   clean_send_node(p_node);
 *   add_send_node(p_node, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
 *   return X_MALLOC_FAILED;
 *   }
 */
void entry_init(void)
{
        if (g_locate_cfg == NULL) {
                g_locate_cfg = (locate_cfg_t *)malloc(sizeof(locate_cfg_t));
                memset(g_locate_cfg, 0, sizeof(locate_cfg_t));
        }
         g_kv_cache = kv_cache_create(8);
	pmr_load_data_all();
        seg_file_load(g_locate_cfg->seg_file_name);
        map_topo_load();
}

locate_cfg_t *locate_cfg_init(char *p_name)
{
        locate_cfg_t *p_locate_cfg = (locate_cfg_t *)malloc(sizeof(locate_cfg_t));
        memset(p_locate_cfg, 0, sizeof(locate_cfg_t));

        const char              *str_val = NULL;
        struct json_object      *obj = NULL;
        struct json_object      *new_obj = NULL;

        struct json_object *cfg = json_object_from_file(p_name);

        if (cfg == NULL) {
                goto fail;
        }

        if (json_object_object_get_ex(cfg, "mapsegment", &obj)) {
                if (json_object_object_get_ex(obj, "map_seg_file", &new_obj)) {
                        str_val = json_object_get_string(new_obj);
                        strcpy(p_locate_cfg->seg_file_name, str_val);
                } else { goto fail; }
        } else { goto fail; }

        if (json_object_object_get_ex(cfg, "mlocate", &obj)) {
                if (json_object_object_get_ex(obj, "WEIGHT_NO", &new_obj)) {            //无连接权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_no = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_LAST_SAM", &new_obj)) {   //道路相同权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_last_sam = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_CONN_DIRECT", &new_obj)) {  //直接相连权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_conn_direct = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_CONN_INDIRECT", &new_obj)) {  //间接相连权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_conn_indirect = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_BYROAD", &new_obj)) {    //经过辅路权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_byroad = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_NEAEST", &new_obj)) {    //最近道路权重值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_neaest = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_MIN_SUB", &new_obj)) {    //方向角夹角最小权重
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_min_sub = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_SPEED", &new_obj)) {    //速度权重
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_speed = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_ALT", &new_obj)) {  //海拔权重
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_alt = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_INDIRECT_IN_NODE", &new_obj)) { //间接相连，通过连接点权重
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_indirect_in_node = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "GRID_EXTEND_LIMIT", &new_obj)) {  //BL筛选 网格拓宽值
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->grid_extend_limit = strtod(str_val, NULL);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "PMR_MAX_DIST", &new_obj)) {  //line筛选，最大距离
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->pmr_max_dist = atoi(str_val);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "PMR_MAX_SUB_DIR", &new_obj)) {  //line筛选，最大夹角
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->pmr_max_sub_dir = atoi(str_val);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "TRACK_BACK_DEEP", &new_obj)) {  //向前追溯道路深度
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->track_back_deep = atoi(str_val);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_DIST_LIMIT", &new_obj)) {  //满足此条件，才进行最近距离加成
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_dist_limit = atoi(str_val);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "WEIGHT_DIR_LIMIT", &new_obj)) { //满足此条件，才进行最小夹角加成
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->weight_dir_limit = atoi(str_val);
                 } else { goto fail; }

                if (json_object_object_get_ex(obj, "NODE_RANGE_DIST", &new_obj)) { //判断是否再辅路附近距离
                        str_val = json_object_get_string(new_obj);
                        p_locate_cfg->node_range_dist = atoi(str_val);
                 } else { goto fail; }
        } else { goto fail; }

        return p_locate_cfg;

fail:
        x_printf(E, "invalid config file :%s\n", p_name);
        exit(1);
}



