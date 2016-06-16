#include "pmr_qlocate.h"
#include "map_pmr.h"
#include "pmr_utils.h"

int entry_cmd_qlocate(struct data_node *p_node)
{
#if 0
	if (!p_node) {
		return 0;
	}

	map_line_info   *line;
	double          longitude = 0.0;
	double          latitude = 0.0;
	short           direction = -1;
	unsigned int    line_id = 0;
	struct cache    *p_cache = &p_node->mdl_send.cache;

	if (0 != check_parameter_qlocate(p_node, &longitude, &latitude, &direction, &line_id)) {
		return 0;
	}

	x_printf(I, "lon:%f, lat:%f, dir:%d\n", longitude, latitude, direction);

	if (line_id > 0) {	/* quick location*/
		line = map_line_query(line_id);

		if (!line) {
			cache_add(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
			return 0;
		}
	} else {
		int ret = pmr_locate(&line, direction, longitude, latitude);

		if (0 != ret) {
			cache_add(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
			return 0;
		}
	}

	// *2\r\n$6\r\nlineID\r\n$5\r\n12345\r\n
	cache_add(p_cache, "*", 1);
	put_number_out(p_cache, 6);

	//       cache_add( p_cache, "$", 1 );
	//       put_number_out(p_cache, 6);
	//       cache_add( p_cache, "RRID", 4);
	//       cache_add( p_cache, "\r\n", 2);

	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->rr_id));
	put_number_out(p_cache, line->rr_id);

	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->sgid));
	put_number_out(p_cache, line->sgid);

	// TFID
	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->tfid));
	put_number_out(p_cache, line->tfid);

	// countyCode
	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->countyCode));
	put_number_out(p_cache, line->countyCode);

	// RT
	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->rt));
	put_number_out(p_cache, line->rt);

	// line_id
	cache_add(p_cache, "$", 1);
	put_number_out(p_cache, get_number_len(line->line_id));
	put_number_out(p_cache, line->line_id);
#endif /* if 0 */
	return 0;
}

