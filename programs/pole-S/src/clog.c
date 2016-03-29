#include "clog.h"

int log_ctx_init(const char *path, int level)
{
	const SLogLevelT *plevel = SLogIntegerToLevel((short)level);

	if (plevel && SLogOpen(path, plevel)) {
		return 0;
	}

	return -1;
}

