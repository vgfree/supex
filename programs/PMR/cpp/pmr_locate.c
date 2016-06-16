#include "pmr_locate.h"
#include "map_pmr.h"
#include "map_seg.h"
#include "pmr_utils.h"

int entry_cmd_locate(struct data_node *p_node)
{
	if (!p_node) {
		return 0;
	}

	map_line_info   *line;
	double          longitude = 0.0;
	double          latitude = 0.0;
	short           direction = -1;
	struct cache    *p_cache = &p_node->mdl_send.cache;

	if (0 != check_parameter_locate(p_node, &longitude, &latitude, &direction)) {
		return 0;
	}

	x_printf(I, "lon:%f, lat:%f, dir:%d\n", longitude, latitude, direction);

	int ret = pmr_locate(&line, direction, longitude, latitude);

	if (0 != ret) {
		cache_append(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
		return 0;
	}

	back_seg ret_seg;

	if (0 != map_seg_query(line->rr_id, line->sgid, &ret_seg)) {
		cache_append(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
		return 0;
	}

	// *2\r\n$6\r\nlineID\r\n$5\r\n12345\r\n
	cache_append(p_cache, "*", 1);
	put_number_out(p_cache, 11);

	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->rr_id));
	put_number_out(p_cache, line->rr_id);

	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->sgid));
	put_number_out(p_cache, line->sgid);

	// TFID
	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->tfid));
	put_number_out(p_cache, line->tfid);

	// countyCode  TODO  need init the data file
	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(ret_seg.ptr_seg->countyCode));
	put_number_out(p_cache, ret_seg.ptr_seg->countyCode);

	// RT
	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(ret_seg.ptr_seg->sgid_rt));
	put_number_out(p_cache, ret_seg.ptr_seg->sgid_rt);

	// roadName
	char road_name[128 + 1] = { 0 };

	if (ret_seg.ptr_name) {
		strncpy(road_name, ret_seg.ptr_name, 128);
	}

	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, strlen(road_name));
	put_string_out(p_cache, road_name);

	// 起始经纬度
	put_double_out(p_cache, ret_seg.ptr_seg->start_lon);
	put_double_out(p_cache, ret_seg.ptr_seg->start_lat);
	put_double_out(p_cache, ret_seg.ptr_seg->end_lon);
	put_double_out(p_cache, ret_seg.ptr_seg->end_lat);

	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(ret_seg.ptr_seg->length));
	put_number_out(p_cache, ret_seg.ptr_seg->length);

	return 0;
}

