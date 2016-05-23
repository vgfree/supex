/*                                                                                                      
 * Author       : shumenghui@mirrtalk.com
 * Date         : 2016-05-12
 * Function     : encrypt the id
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/evp.h>                                                                                                                                                                                           
#include <openssl/aes.h>
#include <openssl/err.h>

#include "id_encrypt.h"
#include "utils.h"

/* A 256 bit key */
static const unsigned char key[] = "11234567890123456789012345678900";

/* A 128 bit IV */
static const unsigned char iv[] = "5123456789012340";

/* Some additional data to be authenticated */
static const unsigned char aad[] = "mirrtalk";

static int extraction(unsigned char *cipher, int len, unsigned char *fibonacci_text)
{
        if(len < 34) {
                x_printf(E, "fibonacci_text len less than 34 %d", len);
                return -1;
        }
        int f1 = 0, f2 = 1, f3 = 2, f4 = 3, f5 = 5, f6 = 8, f7 = 13, f8 = 21, f9 = 34;
        //x_printf(D, "after fibobacci %x %x %x %x %x %x %x %x %x %x %x\n", cipher[f1], cipher[f2], cipher[f3], cipher[f4], cipher[f5], cipher[f6], cipher[len-2], cipher[f7], cipher[f8], cipher[f9], cipher[len-1]);
        //snprintf(fibonacci_text, 38, "%x%x%x%x%x%x%x%x%x%x%x", cipher[f1], cipher[f2], cipher[f3], cipher[f4], cipher[f5], cipher[f6], cipher[len-2], cipher[f7], cipher[f8], cipher[f9], cipher[len-1]);        
        x_printf(D, "after fibobacci %x %x %x %x %x %x %x %x %x %x\n", cipher[f1], cipher[f2], cipher[f3], cipher[f4], cipher[f5], cipher[f6], cipher[f7], cipher[f8], cipher[f9], cipher[len-1]);
        snprintf(fibonacci_text, 38, "%x%x%x%x%x%x%x%x%x%X", cipher[f1], cipher[f2], cipher[f3], cipher[f4], cipher[f5], cipher[f6], cipher[f7], cipher[f8], cipher[f9], cipher[len-1]);
        return 0;
}

static int id_cmd_in(kv_handler_t *handler, const char *id, long date, const unsigned char *cipher)
{       
        if(!id || !cipher) {
                x_printf(E, "id_cmd_in err\n");
                return -1;
        }

        char cmd[CMD_BUF_SIZE] = { '\0' };

        snprintf(cmd, CMD_BUF_SIZE, "hmset %s date %ld  cipher %s", id, date, cipher);

        kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

        if (ans->errnum != ERR_NONE) {
                kv_answer_release(ans);
                x_printf(E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
                return -1;          
        }

        kv_answer_release(ans);
        return 0;
}

static int id_cmd_del(kv_handler_t *handler, const char *id)
{
        if(!id) {
                x_printf(E, "id_cmd_in err\n");
                return -1;
        }

        char cmd[CMD_BUF_SIZE] = { '\0' };

        snprintf(cmd, CMD_BUF_SIZE, "del %s", id);

        kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

        if (ans->errnum != ERR_NONE) {
                kv_answer_release(ans);
                x_printf(E, "command[%s] --> errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
                return -1;          
        }

        kv_answer_release(ans);
        return 0;

}

int id_cmd_out(kv_handler_t *handler, const char *id, long date, unsigned char *cipherbuff, unsigned char *encrypt_text)
{      
        if (!handler || !id) {
                return -1;
        }

        int flag = 0;
        char cmd[CMD_BUF_SIZE] = { 0 };
        /* Buffer for the tag */
        unsigned char tag[16];
        //unsigned char iv_date[16] = "";
        unsigned char iv_date[16] = "";
        int ciphertext_len = 0;
        unsigned char plaintext[128] = "";
        //unsigned char encrypt_text[64] = "";
        snprintf(cmd, CMD_BUF_SIZE, "hget %s date",id);
        kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

        if (ans->errnum != ERR_NONE) {
                if (ans->errnum == ERR_NIL) {
                        snprintf(plaintext, 128, "%s%ld", id, date);
                        /* Encrypt the plaintext */
                        snprintf(iv_date, 16, "%ld", date);
                        ciphertext_len = id_encrypt(plaintext, strlen(plaintext), aad, strlen(aad), key, iv_date, cipherbuff, tag);
                        //printf("Ciphertext is: %d\n", ciphertext_len);
                        //BIO_dump_fp(stdout, cipherbuff, ciphertext_len);
                        /*int i = 0;
                        int k = 0;
                        for(i=0; i<ciphertext_len; i+=2) {
                                encrypt_text[k++] = cipherbuff[i];
                        }
                        */
                        extraction(cipherbuff, ciphertext_len, encrypt_text);
                        id_cmd_in(handler, id, date, encrypt_text);
                        //printf("encrypt_text is: %d\n", strlen(encrypt_text));
                        //BIO_dump_fp(stdout, encrypt_text, strlen(encrypt_text));
                        flag = 1;
                        goto end;
                }
                else {
                        x_printf(E, "cmd:%s, errnum:%d, errstr:%s\n", cmd, ans->errnum, ans->err);
                        flag = -1;
                        goto end;
                }
        }

        unsigned long len = kv_answer_length(ans);

        if (len == 1) {
                kv_answer_value_t *value = kv_answer_first_value(ans);

                if (!value) {
                        x_printf(E, "Failed to get date by cmd[%s].\n", cmd);
                        flag = -2;
                        goto end;
                }
                x_printf(D, "after executed cmd[%s], date:%s\n", cmd, (char *)value->ptr);
                long get_date = atoi((char *)value->ptr);
                if(date == get_date) {
                        memset(cmd, 0, sizeof(cmd));
                        snprintf(cmd, CMD_BUF_SIZE, "hget %s cipher",id); 
                        kv_answer_t *ans_cipher = kv_ask(handler, cmd, strlen(cmd));
                        if (ans_cipher->errnum != ERR_NONE) {
                                flag = -1;
                                kv_answer_release(ans_cipher);
                                goto end;
                        }
                        unsigned num = kv_answer_length(ans_cipher);
                        if (num == 1) {
                                kv_answer_value_t *cipher_text = kv_answer_first_value(ans_cipher);
                                if (!cipher_text) {
                                        x_printf(E, "Failed to get cipher_text by cmd[%s].\n", cmd);
                                        flag = -2;
                                }
                                else {
                                        memcpy(encrypt_text, (char *)cipher_text->ptr, cipher_text->ptrlen);
                                } 

                        }
                        kv_answer_release(ans_cipher);
                        goto end;
                }
                else if (date > get_date){
                        //id_cmd_del(handler, id);
                        snprintf(plaintext, 128, "%s%ld", id, date);
                        /* Encrypt the plaintext */
                        snprintf(iv_date, 16, "%ld", date);
                        ciphertext_len = id_encrypt(plaintext, strlen(plaintext), aad, strlen(aad), key, iv_date, cipherbuff, tag);
                        //printf("Ciphertext is: %d\n", ciphertext_len);
                        //BIO_dump_fp(stdout, cipherbuff, ciphertext_len);
                        /*int i = 0;
                        int k = 0;
                        for(i=0; i<ciphertext_len; i+=2) {
                                encrypt_text[k++] = cipherbuff[i];
                        }
                        */
                        extraction(cipherbuff, ciphertext_len, encrypt_text);
                        id_cmd_in(handler, id, date, encrypt_text);
                        //printf("encrypt_text is: %d\n", strlen(encrypt_text));
                        //BIO_dump_fp(stdout, encrypt_text, strlen(encrypt_text));
                        flag = 1;
                }
                else {
                        x_printf(E, "now date:%ld < get date:%ld\n", date, get_date);
                        flag = -3;
                }

        }

        kv_answer_release(ans);
        return flag;
end:
        kv_answer_release(ans);
        return flag;
}

void handleErrors(void)
{
        unsigned long errCode;

        x_printf(E, "An error occurred\n");
        while(errCode = ERR_get_error())
        {
                char *err = ERR_error_string(errCode, NULL);
                x_printf(E, "encryption err %s\n", err);
        }
        abort();
}
/*
static void swap_text(unsigned char *plaintext, int plaintext_len)
{
        char temp = plaintext[0];
        plaintext[0] = plaintext[plaintext_len-1];
        plaintext[1] = plaintext[plaintext_len-2];
        plaintext[3] = plaintext[plaintext_len-1];
        plaintext[5] = plaintext[plaintext_len-3];
        plaintext[plaintext_len-2] = temp;
}
*/
int id_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
                int aad_len, unsigned char *key, unsigned char *iv,
                unsigned char *ciphertext, unsigned char *tag)
{
        //swap_text(plaintext, plaintext_len);
        EVP_CIPHER_CTX *ctx = NULL;
        int len = 0, ciphertext_len = 0;

        /* Create and initialise the context */
        if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        /* Initialise the encryption operation. */
        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                handleErrors();

        /* Set IV length if default 12 bytes (96 bits) is not appropriate */
        if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
                handleErrors();

        /* Initialise key and IV */
        if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

        /* Provide any AAD data. This can be called zero or more times as
         * required
         */
        if(aad && aad_len > 0)
        {
                if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
                        handleErrors();
        }

        /* Provide the message to be encrypted, and obtain the encrypted output.
         * EVP_EncryptUpdate can be called multiple times if necessary
         */
        if(plaintext)
        {
                if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
                        handleErrors();

                ciphertext_len = len;
        }

        /* Finalise the encryption. Normally ciphertext bytes may be written at
         * this stage, but this does not occur in GCM mode
         */
        if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
        ciphertext_len += len;

        /* Get the tag */
        if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
                handleErrors();

        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);

        return ciphertext_len;
}

int id_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
                int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
                unsigned char *plaintext)
{
        EVP_CIPHER_CTX *ctx = NULL;
        int len = 0, plaintext_len = 0, ret;

        /* Create and initialise the context */
        if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        /* Initialise the decryption operation. */
        if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                handleErrors();

        /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
        if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
                handleErrors();

        /* Initialise key and IV */
        if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

        /* Provide any AAD data. This can be called zero or more times as
         * required
         */
        if(aad && aad_len > 0)
        {
                if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
                        handleErrors();
        }

        /* Provide the message to be decrypted, and obtain the plaintext output.
         * EVP_DecryptUpdate can be called multiple times if necessary
         */
        if(ciphertext)
        {
                if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
                        handleErrors();

                plaintext_len = len;
        }

        /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
        if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
                handleErrors();

        /* Finalise the decryption. A positive return value indicates success,
         * anything else is a failure - the plaintext is not trustworthy.
         */
        ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);

        if(ret > 0)
        {
                /* Success */
                plaintext_len += len;
                return plaintext_len;
        }
        else
        {
                /* Verify failed */
                return -1;
        }
}
