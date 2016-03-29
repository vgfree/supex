#include "mfptp_utils.h"

int mfptp_aes_ecb_encrypt_evp(char *key, int key_len, char *crypted, char *encrypted, int len)
{
	int             en_len1;
	int             en_len2;
	EVP_CIPHER_CTX  ctx;

	EVP_CIPHER_CTX_init(&ctx);

	int rv;
	rv = EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char *)key, NULL);

	rv = EVP_EncryptUpdate(&ctx, encrypted, &en_len1, crypted, len);

	if (rv != 1) {
		printf("Err\n");
		EVP_CIPHER_CTX_cleanup(&ctx);

		return -1;
	}

	rv = EVP_EncryptFinal_ex(&ctx, encrypted + en_len1, &en_len2);

	if (rv != 1) {
		printf("Err\n");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	EVP_CIPHER_CTX_cleanup(&ctx);

	return en_len1 + en_len2;
}

int mfptp_aes_ecb_decrypt_evp(char *key, int key_len, char *crypted, char *decrypted, int len)
{
	int             de_len1;
	int             de_len2;
	EVP_CIPHER_CTX  ctx;

	EVP_CIPHER_CTX_init(&ctx);

	int ret = EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char *)key, NULL);

	int rv = EVP_DecryptUpdate(&ctx, decrypted, &de_len1, crypted, len);

	if (rv != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	rv = EVP_DecryptFinal_ex(&ctx, decrypted + de_len1, &de_len2);

	if (rv != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	EVP_CIPHER_CTX_cleanup(&ctx);

	return de_len1 + de_len2;
}

int mfptp_idea_ecb_encrypt_evp(char *key, int key_len, char *crypted, char *encrypted, int len)
{
	int             en_len1;
	int             en_len2;
	EVP_CIPHER_CTX  ctx;

	EVP_CIPHER_CTX_init(&ctx);
	int rv;
	rv = EVP_EncryptInit_ex(&ctx, EVP_idea_ecb(), NULL, (const unsigned char *)key, NULL);
	rv = EVP_EncryptUpdate(&ctx, encrypted, &en_len1, crypted, len);

	if (rv != 1) {
		printf("Err\n");
		EVP_CIPHER_CTX_cleanup(&ctx);

		return -1;
	}

	rv = EVP_EncryptFinal_ex(&ctx, encrypted + en_len1, &en_len2);

	if (rv != 1) {
		printf("Err\n");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	EVP_CIPHER_CTX_cleanup(&ctx);

	return en_len1 + en_len2;
}

int mfptp_idea_ecb_decrypt_evp(char *key, int key_len, char *crypted, char *decrypted, int len)
{
	int             de_len1;
	int             de_len2;
	EVP_CIPHER_CTX  ctx;

	EVP_CIPHER_CTX_init(&ctx);
	int     ret = EVP_DecryptInit_ex(&ctx, EVP_idea_ecb(), NULL, (const unsigned char *)key, NULL);
	int     rv = EVP_DecryptUpdate(&ctx, decrypted, &de_len1, crypted, len);

	if (rv != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	printf("de_len1 = %d \n", de_len1);

	rv = EVP_DecryptFinal_ex(&ctx, decrypted + de_len1, &de_len2);

	if (rv != 1) {
		printf("idea\n");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	EVP_CIPHER_CTX_cleanup(&ctx);

	return de_len1 + de_len2;
}

int mfptp_ungzip(char *gz_buf, int gzlen, char *ungz_buf, int ungz_buf_len)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != inflateInit(&s)) {
		return -1;
	}

	s.next_in = gz_buf;
	s.avail_in = gzlen;
	s.next_out = ungz_buf;
	s.avail_out = ungz_buf_len;

	if (Z_STREAM_END != inflate(&s, Z_FINISH)) {
		return -1;
	}

	if (Z_OK != inflateEnd(&s)) {
		return -1;
	}

	return s.total_out;
}

int mfptp_gzip(char *buf_def, int len_def, char *buf_undef, int len_undef)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != deflateInit(&s, 6)) {
		return -1;
	}

	s.next_in = buf_undef;
	s.avail_in = len_undef;
	s.next_out = buf_def;
	s.avail_out = len_def;
	int ret;

	if (Z_STREAM_END != deflate(&s, Z_FINISH)) {
		return -1;
	}

	if (Z_OK != deflateEnd(&s)) {
		return -1;
	}

	return s.total_out;
}

int mfptp_unzip(char *z_buf, int zlen, char *unz_buf, int unz_buf_len)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != inflateInit2(&s, MAX_WBITS + 16)) {
		printf("a\n");
		return -1;
	}

	s.next_in = z_buf;
	s.avail_in = zlen;
	s.next_out = unz_buf;
	s.avail_out = unz_buf_len;

	if (Z_STREAM_END != inflate(&s, Z_FINISH)) {
		printf("s\n");
		return -1;
	}

	if (Z_OK != inflateEnd(&s)) {
		printf("d\n");
		return -1;
	}

	return s.total_out;
}

int mfptp_zip(char *buf_def, int len_def, char *buf_undef, int len_undef)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != deflateInit2(&s, 6, 8, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY)) {
		return -1;
	}

	s.next_in = buf_undef;
	s.avail_in = len_undef;
	s.next_out = buf_def;
	s.avail_out = len_def;
	int ret;

	if (Z_STREAM_END != deflate(&s, Z_FINISH)) {
		return -1;
	}

	if (Z_OK != deflateEnd(&s)) {
		return -1;
	}

	return s.total_out;
}

void mfptp_set_user_secret_key(struct mfptp_key *p_info)
{
	if (1 == RAND_bytes(p_info->key, 16)) {} else {
		int i = 0;

		for (; i < 16; i++) {
			p_info->key[i] = rand() % 255;
		}
	}
}

