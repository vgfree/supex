#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "traffic_draw.h"
#include "loadgps.h"

int main(int argc, char *argv[])
{
        if(argc < 6) {
                printf("need 4 parameter: path name weight hight\n");
                return -1;
        }
        sg_png_manag_t p_manage;
        memset(&p_manage, 0, sizeof(sg_png_manag_t));
        initList(&(p_manage.cur_sg));
        struct  timeval  start;
        struct  timeval  end;
        unsigned long timer;

        gettimeofday(&start,NULL);
        printf("--draw start--\n");
        loadgps_form_tablefile(argv[5], "surrounding_road", &p_manage);
        printf("-------load gps end %d\n", p_manage.sg_count);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("timer load = %ld us\n",timer);

        traffic_draw_start(&p_manage, argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
        printf("over: %s%s\n", argv[1], argv[2]);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("timer whole = %ld us\n",timer);
        return 0;
}
