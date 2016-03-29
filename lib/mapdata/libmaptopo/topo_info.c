#include "topo_info.h"
#include "map_errno.h"
#include "libmini.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int map_topo_manager_init_buff(map_topo_manager_t *p_manage, uint16_t load_once, uint32_t max_rrid)
{
        if(!p_manage || max_rrid <=0 || load_once <= 0)
                return ERR_PMR_PARAMETER;

        p_manage->max_rr_id = max_rrid;
        p_manage->load_once = load_once;

        uint16_t ret = max_rrid % p_manage->file_cfg.rrid_index_len;
        uint16_t len = max_rrid / p_manage->file_cfg.rrid_index_len;
        p_manage->rrid_buff_len = (ret == 0) ? len : (len + 1);
        p_manage->p_manage_buff = (topo_info_rr_t **)calloc(p_manage->rrid_buff_len, (sizeof(topo_info_rr_t *)));

        int idx = 0;
        for(idx = 0; idx < p_manage->rrid_buff_len; idx ++) {
                topo_info_rr_t *p_rr = (topo_info_rr_t *)calloc(p_manage->file_cfg.rrid_index_len, sizeof(topo_info_rr_t));
                p_manage->p_manage_buff[idx] = p_rr;
        }

        return 0;
}

static int topo_info_add_line(topo_info_t *p_topo, topo_file_line_t *p_line)
{
        if(!p_topo || !p_line)
                return -1;

        if(!p_topo->p_conn_buff) {
                p_topo->rrid = p_line->rrid1;
                p_topo->sgid = p_line->sgid1;
                p_topo->max_count = p_line->conn_count;
                p_topo->p_conn_buff = (topo_conn_sg_t *)calloc(p_line->conn_count, sizeof(topo_conn_sg_t));
                p_topo->conn_count = 0;
        }

        if(p_topo->conn_count >= p_topo->max_count) {
                x_printf(E,"conn_count >= max_count, id:%d\n", p_line->id);
                return -1;
        }
        topo_conn_sg_t *p_conn = p_topo->p_conn_buff + p_topo->conn_count;
        p_conn->rrid = p_line->rrid2;
        p_conn->sgid = p_line->sgid2;
        p_conn->conn_type = p_line->conn_type;

        if(p_line->conn_type == TOPO_CONN_TYPE_BYROAD) {
                p_conn->conn_node.L1 = p_line->SL;
                p_conn->conn_node.B1 = p_line->SB;
                p_conn->conn_node.L2 = p_line->EL;
                p_conn->conn_node.B2 = p_line->EB;
        }
        p_topo->conn_count++;

        return 0;
}

topo_info_t* map_topo_get_conn(map_topo_manager_t *p_manager, uint32_t rrid, uint8_t sgid)
{
        uint16_t rr_idx1 = rrid / p_manager->file_cfg.rrid_index_len;
        uint16_t rr_idx2 = rrid % p_manager->file_cfg.rrid_index_len;
        topo_info_rr_t *p_topo_rr_buf = p_manager->p_manage_buff[rr_idx1];
        topo_info_rr_t *p_topo_rr = p_topo_rr_buf + rr_idx2;

        if(!p_topo_rr)
                return NULL;

        if(sgid > p_topo_rr->sg_count) {
                x_printf(E, "sgid > count1, rrid:%d", rrid);
                return NULL;
        }

        topo_info_t *p_topo = p_topo_rr->p_topo_buf + (sgid - 1);

        return p_topo;
}

int map_topo_manager_add_line(map_topo_manager_t *p_manager, topo_file_line_t *p_line)
{
        if(!p_manager || !p_line)
                return -1;

        uint32_t rrid = p_line->rrid1;
        uint8_t sgid = p_line->sgid1;


        uint16_t rr_idx1 = rrid / p_manager->file_cfg.rrid_index_len;
        uint16_t rr_idx2 = rrid % p_manager->file_cfg.rrid_index_len;
        topo_info_rr_t *p_topo_rr_buf = p_manager->p_manage_buff[rr_idx1];
        topo_info_rr_t *p_topo_rr = p_topo_rr_buf + rr_idx2;
        if(!p_topo_rr)
                return -1;

        if(!p_topo_rr->p_topo_buf) {
                p_topo_rr->rrid = rrid;
                p_topo_rr->sg_count = p_line->count1;
                p_topo_rr->p_topo_buf = (topo_info_t *)calloc(p_line->count1, sizeof(topo_info_t));
        }

        if(sgid > p_topo_rr->sg_count) {
                x_printf(E, "sgid > count1, rrid:%d", rrid);
                return -1;
        }

        topo_info_t *p_topo = p_topo_rr->p_topo_buf + (sgid - 1);
        topo_info_add_line(p_topo, p_line);

        return 0;
}

