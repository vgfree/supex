#include "map_poi.h"
#include "map_poi_file.h"
#include "poi_cfg.h"
#include "libmini.h"

#include <string.h>
#include <stdlib.h>

static mappoi_manager g_manage;

int mappoi_load_cfg(char *cfg_file_name)
{
	if (!cfg_file_name) {
		return -1;
	}

	mappoi_cfg_file cfg;
	memset(&cfg, 0, sizeof(cfg));

	if (0 != mappoi_load_cfg_data(&cfg, cfg_file_name)) {
		return -1;
	}

	strncpy(g_manage.file_name_poi, cfg.file_name_poi, sizeof(g_manage.file_name_poi));
	strncpy(g_manage.file_name_sg, cfg.file_name_sg, sizeof(g_manage.file_name_sg));

	return 0;
}

int mappoi_load()
{
	if ((strlen(g_manage.file_name_poi) <= 0) || (strlen(g_manage.file_name_sg) <= 0)) {
		return -1;
	}

	mappoi_load_file(&g_manage);
	return 0;
}

mappoi_poi_buf *mappoi_query_poi(unsigned int rr_id, unsigned int sg_id)
{
	int     rr_idx = rr_id / g_manage.load_once;
	int     rr_count = rr_id % g_manage.load_once;

	mappoi_rr *p_rr = g_manage.rr_buf[rr_idx] + rr_count;

	if (!p_rr) {
		x_printf(D, "mappoi_query_poi rr_id ERROR,rr_id:%d,sg_id:%d", rr_id, sg_id);
		return NULL;
	}

	if (p_rr->rr_id != rr_id) {
		x_printf(D, "mappoi_query_poi rr_id ERROR,rr_id:%d,sg_id:%d", rr_id, sg_id);
		return NULL;
	}

	if (sg_id >= p_rr->sg.sg_count) {
		x_printf(D, "mappoi_query_poi sg_id ERROR,rr_id:%d,sg_id:%d", rr_id, sg_id);
		return NULL;
	}

	mappoi_sg *p_sg = (p_rr->sg.sg_buf) + (sg_id - 1);

	if (!p_sg || (p_sg->poi_buf.poi == NULL) || (p_sg->poi_buf.poi_idx == 0)) {
		x_printf(D, "mappoi_query_poi poi ERROR,rr_id:%d,sg_id:%d", rr_id, sg_id);
		return NULL;
	}

	return &(p_sg->poi_buf);
}

mappoi_iterator *mappoi_iterator_init(unsigned int rr_id, unsigned int sg_id, int sg_max)
{
	mappoi_iterator *p_it = (mappoi_iterator *)calloc(1, sizeof(mappoi_iterator));

	p_it->rr_id = rr_id;
	p_it->sg_id = sg_id;
	p_it->sg_max = sg_max;
	p_it->sg_count = 0;

	return p_it;
}

int mappoi_iterator_next(mappoi_iterator *p_it, mappoi_poi_buf **poi_buf)
{
	if (p_it->sg_max <= 0) {
		return 1;
	}

	// printf("rr_id:%d,",p_it->rr_id);
	//  printf("sg_id:%d,",p_it->sg_id);
	//  printf("sg_max:%d,",p_it->sg_max);
	//  printf("sg_count:%d,",p_it->sg_count);

	*poi_buf = mappoi_query_poi(p_it->rr_id, p_it->sg_id);
	// if(!(*poi_buf))
	// poi_buf = NULL;

	int             rr_idx = (p_it->rr_id) / (g_manage.load_once);
	int             rr_count = (p_it->rr_id) % (g_manage.load_once);
	mappoi_sg       temp = ((((g_manage.rr_buf)[rr_idx])[rr_count]).sg.sg_buf)[p_it->sg_id - 1];

	p_it->sg_count = temp.sg_count;

	if ((temp.next_rr_id == 0) && (temp.next_sg_id == 0)) {
		if (p_it->sg_id < p_it->sg_count) {
			p_it->sg_id = p_it->sg_id + 1;
			// printf("**0 in**");
		} else if (p_it->sg_id == p_it->sg_count) {
			p_it->sg_max = 0;
			// printf("**0 end***");
		}
	}

	if ((0 != temp.next_rr_id) && (0 != temp.next_sg_id)) {
		p_it->rr_id = temp.next_rr_id;
		p_it->sg_id = temp.next_sg_id;
		// printf("**NO 0***");
	}

	p_it->sg_max = p_it->sg_max - 1;

	if (!(*poi_buf)) {
		poi_buf = NULL;
		// printf("#####\n");
		return 0;
	}

	return 0;
}

void mappoi_iterator_destory(mappoi_iterator *iterator)
{
	if (iterator != NULL) {
		free(iterator);
	}
}

