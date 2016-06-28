#ifndef _BUSI_W2FILE_
#define _BUSI_W2FILE_

#include "register.h"

static FILE *g_fp = NULL;

int w2file_init(struct pole_conf *conf)
{
	char fname[256] = "business.db";

	g_fp = fopen(fname, "a+");

	if (!g_fp) {
		x_printf(E, "fopen:('%s', 'r+') fail. Error-%s.\n", fname, strerror(errno));
		return -1;
	}

	x_printf(I, "Open w2file <%s> succeed.\n", fname);
	return 0;
}

int w2file_done(void *args, size_t arg_size)
{
	if (!g_fp) {
		return -1;
	}

	int res = fwrite(args, arg_size, 1, g_fp);

	if (res != 1) {
		x_printf(E, "fwrite:(%s) fail. Error-%s.\n", (char *)args, strerror(errno));
		return BUSI_ERR_FAIL;
	}

	fflush(g_fp);
	return 0;
}

void w2file_destroy()
{
	if (g_fp) {
		fflush(g_fp);
		fclose(g_fp);
	}

	x_printf(I, "Close the w2file file pointer.\n");
}
#endif	/* ifndef _BUSI_W2FILE_ */

