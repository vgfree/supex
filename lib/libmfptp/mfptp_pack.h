#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mfptp_utils.h"

#define MFPTP_PACK_INIT         0
#define MFPTP_PACK_HEAD         1
#define MFPTP_PACK_PACKAGE      2
#define MFPTP_PACK_OVER         3
#define MFPTP_PACK_ERROR        4

struct mfptp_pack
{
	short                   version;
	short                   sub_version;
	unsigned char           method;
	unsigned char           compress;
	unsigned char           encrypt;
	unsigned char           packages;
	int                     step;	/*-parse step-*/
	int                     doptr;	/*-pointer now pos-*/
	int                     index;
	struct mfptp_key        m_key;
};

void mfptp_init_pack_info(struct mfptp_pack *p_info);

void mfptp_pack_set_packages(int count, struct mfptp_pack *p_info);

int mfptp_pack_frames_with_packages(char *src, int len, char *dst, int more, struct mfptp_pack *p_info);

int mfptp_pack_frame(char *src, int len, char *dst, int more, struct mfptp_pack *p_info);

int mfptp_pack_hdr(char *dst, struct mfptp_pack *p_info);

int mfptp_pack_len(int len);

