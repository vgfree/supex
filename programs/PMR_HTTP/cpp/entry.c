#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "entry.h"
#include "net_cache.h"
#include "map_seg.h"
#include "map_pmr.h"

void entry_init(void)
{
	pmr_load_data_all();
}

static int get_number_len(uint64_t len)
{
	int i = 1;

	while ((len /= 10) >= 1) {
		i++;
	}

	return i;
}

static int put_number_out(struct net_cache *p_cache, uint64_t nb)
{
	char tmp[32] = { 0 };

	sprintf(tmp, "%zu\r\n", nb);
	return cache_add(p_cache, tmp, strnlen(tmp, 32));
}

static void send_error(struct net_cache *p_cache, char *p_error)
{
	char *p_msg;

	if (!p_error) {
		p_msg = "ERROR";
	} else {
		p_msg = p_error;
	}

	cache_add(p_cache, "-", 1);
	cache_add(p_cache, p_msg, strnlen(p_msg, 32));
	cache_add(p_cache, "\r\n", 2);
}

int entry_cmd_roadrank(struct data_node *p_node)
{
	return 0;
}

int entry_cmd_front_traffic(struct data_node *p_node)
{
	return 0;
}

int entry_cmd_road_traffic(struct data_node *p_node)
{
	return 0;
}

