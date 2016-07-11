/* @author: qianye@mirrtalk.com */

#ifndef MTTPSVP_LIBKV_H
#define MTTPSVP_LIBKV_H

#include <stdint.h>

void mfptpSvp_libkv_init();

void mfptpSvp_libkv_destory();

void mfptpSvp_libkv_set(uint64_t cid, int sfd, const char *v);

int mfptpSvp_libkv_check_handshake(uint64_t cid, int sfd, const char *v, size_t vlen);

void mfptpSvp_libkv_del(uint64_t cid, int sfd);
#endif	/* MTTPSVP_LIBKV_H */

