#ifndef __PKG_SEC_H__
#define __PKG_SEC_H__

#include "pub_incl.h"

unsigned sec_key_get(void);

void sec_encrypt(unsigned key, unsigned char *pkg, int len);

void sec_decrypt(unsigned key, unsigned char *pkg, int len);
#endif

