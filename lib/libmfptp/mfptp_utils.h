#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/idea.h>
#include <openssl/objects.h>
#include <openssl/aes.h>
#include <zlib.h>
#include "zconf.h"

typedef unsigned char uint8_t;

struct mfptp_key
{
	char key[18];
};

int mfptp_aes_ecb_encrypt_evp(char *key, int key_len, char *crypted, char *encrypted, int len);

int mfptp_aes_ecb_decrypt_evp(char *key, int key_len, char *crypted, char *decrypted, int len);

int mfptp_idea_ecb_encrypt_evp(char *key, int key_len, char *crypted, char *encrypted, int len);

int mfptp_idea_ecb_decrypt_evp(char *key, int key_len, char *crypted, char *decrypted, int len);

int mfptp_ungzip(char *gz_buf, int gzlen, char *ungz_buf, int ungz_buf_len);

int mfptp_gzip(char *buf_def, int len_def, char *buf_undef, int len_undef);

int mfptp_unzip(char *z_buf, int zlen, char *unz_buf, int unz_buf_len);

int mfptp_zip(char *buf_def, int len_def, char *buf_undef, int len_undef);

void mfptp_set_user_secret_key(struct mfptp_key *p_info);

