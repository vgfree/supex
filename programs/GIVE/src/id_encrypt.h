#pragma once

#include "libkv.h"
#define CMD_BUF_SIZE 80

#ifdef __cplusplus
extern "C" {
#endif

void handleErrors(void);

int id_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
	int aad_len, unsigned char *key, unsigned char *iv,
	unsigned char *ciphertext, unsigned char *tag);

int id_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
	int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
	unsigned char *plaintext);

int id_cmd_out(kv_handler_t *handler, const char *id, long date, unsigned char *cipherbuff, unsigned char *encrypt_text);

#ifdef __cplusplus
}
#endif

