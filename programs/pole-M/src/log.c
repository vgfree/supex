#include "log.h"

extern int log_init(const char *path, int level)
{
	const SLogLevelT *level_obj = SLogIntegerToLevel((short)level);

	if (level_obj && SLogOpen(path, level_obj)) {
		return 0;
	}

	return -1;
}

