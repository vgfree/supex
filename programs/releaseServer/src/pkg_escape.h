#ifndef __ESCAPE_H__
#define __ESCAPE_H__

#include "pub_incl.h"

int escape(unsigned char *dst, char src);

int unescape(unsigned char *dst, unsigned char *src, int slen);
#endif

