#include "pkg_sec.h"

static const unsigned   M1 = 851027125;
static const unsigned   IA1 = 410213143;
static const unsigned   IC1 = 219143235;

unsigned sec_key_get(void)
{
	srand((unsigned)time(NULL));

	return (unsigned)rand();
}

void sec_encrypt(unsigned key, unsigned char *pkg, int len)
{
	unsigned        idx = 0;
	unsigned        mkey = M1;

	if (key == 0) {
		key = 1;
	}

	if (mkey == 0) {
		mkey = 1;
	}

	while (idx < len) {
		key = IA1 * (key % mkey) + IC1;
		pkg[idx++] ^= (char)((key >> 20) & 0xFF);
	}
}

void sec_decrypt(unsigned key, unsigned char *pkg, int len)
{
	sec_encrypt(key, pkg, len);
}

