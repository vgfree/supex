#include "pmr_utils.h"

int get_number_len(uint64_t len)
{
	int i = 1;

	while ((len /= 10) >= 1) {
		i++;
	}

	return i;
}

int put_number_out(struct cache *p_cache, uint64_t nb)
{
	char tmp[32] = { 0 };

	sprintf(tmp, "%zu\r\n", nb);
	return cache_append(p_cache, tmp, strnlen(tmp, 32));
}

int put_string_out(struct cache *p_cache, char *str)
{
	char tmp[128] = { 0 };

	sprintf(tmp, "%s\r\n", str);
	return cache_append(p_cache, tmp, strnlen(tmp, 128));
}

int put_double_out(struct cache *p_cache, double d)
{
	char tmp[128] = { 0 };

	sprintf(tmp, "%f", d);
	int len = strlen(tmp);
	tmp[len] = '\r';
	tmp[len + 1] = '\n';

	cache_append(p_cache, "$", 1);
	put_number_out(p_cache, len);
	return cache_append(p_cache, tmp, strnlen(tmp, 128));
}

void send_error(struct cache *p_cache, char *p_error)
{
	char *p_msg;

	if (!p_error) {
		p_msg = "ERROR";
	} else {
		p_msg = p_error;
	}

	cache_append(p_cache, "-", 1);
	cache_append(p_cache, p_msg, strnlen(p_msg, 32));
	cache_append(p_cache, "\r\n", 2);
}

int check_parameter_locate(struct data_node *p_node, double *p_lon, double *p_lat, short *p_dir)
{
	struct cache            *p_cache = &p_node->mdl_send.cache;
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (!p_node || !p_buf) {
		return -1;
	}

	if (p_rst->fields != 3) {
		send_error(p_cache, "paramters error");
		return -1;
	}

	if (!p_lon) {
		send_error(p_cache, "longitude error");
		return -1;
	} else {
		double lon = strtod(p_buf + p_rst->field[0].offset, NULL);

		if ((lon > 180.0) || (lon < -180.0)) {
			send_error(p_cache, "longitude error");
			return -1;
		}

		*p_lon = lon;
	}

	if (!p_lat) {
		send_error(p_cache, "latitude error");
		return -1;
	} else {
		double lat = strtod(p_buf + p_rst->field[1].offset, NULL);

		if ((lat > 90.0) || (lat < -90.0)) {
			send_error(p_cache, "latitude error");
			return -1;
		}

		*p_lat = lat;
	}

	if (!p_dir) {
		send_error(p_cache, "direction error");
		return -1;
	} else {
		long dir = strtol(p_buf + p_rst->field[2].offset, NULL, 0);

		if ((dir > 360.0) || (dir < -1.0)) {
			send_error(p_cache, "direction error");
			return 0;
		}

		*p_dir = (short)dir;
	}

	return 0;
}

