#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "../map_pmr.h"
#include "libmini.h"

int main()
{
	char *config_file = "PMR_conf.json";

	pmr_load_cfg(config_file);
	pmr_load_data_all();

	sleep(2);
	char buf[1024];

	while (1) {
		memset(buf, 0, sizeof(buf));
		gets(buf);

		double  longitude = 0;
		double  latitude = 0;
		short   direction = -2;

		sscanf(buf, "%lf,%lf,%d", &longitude, &latitude, &direction);
		x_printf(D, "lon:%lf, lat:%lf, dir:%ld\n", longitude, latitude, direction);

		struct timeval t_start, t_end;
		gettimeofday(&t_start, NULL);
		map_line_info   *p_line;
		int             ret = pmr_locate(&p_line, direction, longitude, latitude);
		gettimeofday(&t_end, NULL);
		long cost_time = t_end.tv_usec - t_start.tv_usec;

		x_printf(D, "Cost time: %ld us\n", cost_time);

		if ((ret == 0) && p_line) {
			x_printf(D, "locate lineID:%d, sgid:%d,start:%f-%f, end:%f-%f, dir:%d\n",
				p_line->line_id, p_line->sgid, p_line->start_lon, p_line->start_lat,
				p_line->end_lon, p_line->end_lat, p_line->dir);
		} else {
			x_printf(D, "locate failed!\n");
		}
	}

	return 0;
}

