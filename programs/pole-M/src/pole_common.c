#include <string.h>

#include "pole_common.h"

int custom_hash(const char *key, int num, int fix)
{
	unsigned int    hash = 0;
	const char      *p = key;

	if ((key == NULL) || (strncmp(key, "", 1) == 0) || (num <= 0)) {
		return -1;
	}

	for (; *p != '\0'; ++p) {
		hash += *p;
	}

	return hash % num + fix;
}
