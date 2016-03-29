#include "pkg_escape.h"

int escape(unsigned char *dst, char src)
{
	switch (src)
	{
		case 0x5B:
		{
			dst[0] = 0x5A;
			dst[1] = 0x01;
			return 2;
		}

		case 0x5A:
		{
			dst[0] = 0x5A;
			dst[1] = 0x02;
			return 2;
		}

		case 0x5D:
		{
			dst[0] = 0x5E;
			dst[1] = 0x01;
			return 2;
		}

		case 0x5E:
		{
			dst[0] = 0x5E;
			dst[1] = 0x02;
			return 2;
		}

		default:
		{
			dst[0] = src;
			return 1;
		}
	}

	return 1;
}

int unescape(unsigned char *dst, unsigned char *src, int slen)
{
	int     dlen = 0;
	int     i;

	for (i = 0; i < slen; i++) {
		if (src[i] == 0x5A) {
			i++;

			if (src[i] == 0x01) {
				dst[dlen++] = 0x5B;
			} else if (src[i] == 0x02) {
				dst[dlen++] = 0x5A;
			} else {
				dst[dlen++] = src[i];		// 异常
			}
		} else if (src[i] == 0x5E) {
			i++;

			if (src[i] == 0x01) {
				dst[dlen++] = 0x5D;
			} else if (src[i] == 0x02) {
				dst[dlen++] = 0x5E;
			} else {
				dst[dlen++] = src[i];		// 异常
			}
		} else {
			dst[dlen++] = src[i];
		}
	}

	return dlen;
}

