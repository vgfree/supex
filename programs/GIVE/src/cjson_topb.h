#ifndef __CJSON_TOPB__
#define __CJSON_TOPB__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gv_common.h"
#include "count.h"

#define GZIP_BUFF_SIZE 1 << 11
#ifdef __cplusplus
extern "C" {
#endif

int cjson_topb(const char *data, char **result, data_count_t *dt);

int gzpb_to_str(char *value, int data_len);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __CJSON_TOPB__ */

