#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mfptp_pack.h"
#include "mfptp_parser.h"
#include "mfptp_utils.h"
int main()
{
	int i;

	// 设置密钥

	struct mfptp_pack       pack;
	struct mfptp_pack       *p_pack;

	p_pack = &pack;
	mfptp_set_user_secret_key(&p_pack->m_key);

	for (i = 0; i < 16; i++) {
		printf("%d ", p_pack->m_key.key[i]);
	}

	printf("\n");

	// 打包数据

	mfptp_init_pack_info(p_pack);
	pack.method = 0x02;
	pack.compress = 0x01;
	pack.encrypt = 0x01;
	pack.version = 0x00;
	pack.sub_version = 0x01;

	mfptp_pack_set_packages(1, p_pack);

	char    buf[100] = "hello world,welcome to beijing ";
	int     len = strlen(buf);
	printf("%d\n", strlen(buf));
	char *p_buf = buf;

	for (i = 0; i < len; i++) {
		printf("%c ", buf[i]);
	}

	printf("\n");

	char    buf1[1000];
	char    *p_buf1 = buf1;
	char    *p = p_buf1;

	int     ret = 0;
	int     total = 0;
	ret = mfptp_pack_hdr(p_buf1, p_pack);
	p_buf1 = p_buf1 + ret;
	total = total + ret;

	ret = mfptp_pack_frame(p_buf, len, p_buf1, 0, p_pack);
	p_buf1 = p_buf1 + ret;
	total = total + ret;
	printf("total = %d\n", total);

	/*
	 *   ret =  mfptp_pack_frames_with_packages(p_buf,len,p_buf1,1,p_pack);
	 *   if(ret == 3)
	 *   {
	 *     printf("打包结束\n");
	 *   }
	 *   else if(ret == 2 )
	 *   {
	 *     printf("继续打包\n");
	 *   }
	 *   else
	 *   {
	 *     printf("打包error\n");
	 *   }
	 *   ret =  mfptp_pack_frames_with_packages(p_buf,len,p_buf1,0,p_pack);
	 *   if(ret == 3)
	 *   {
	 *     printf("打包结束\n");
	 *   }
	 *   else if(ret == 2 )
	 *   {
	 *     printf("继续打包\n");
	 *   }
	 *   else
	 *   {
	 *     printf("打包error\n");
	 *   }
	 *   int k = p_pack->index;
	 *   printf("%d\n",k);
	 */

	// 解析数据

	struct mfptp_parser_info        mm_parser;
	struct mfptp_parser_info        *p_m_parser;
	p_m_parser = &mm_parser;
	mfptp_init_parser_info(p_m_parser);
	memcpy(p_m_parser->m_key.key, p_pack->m_key.key, 16);

	mfptp_register_callback(p_m_parser, mfptp_drift_out_callback);

	int ret1 = 0;
	ret1 = mfptp_parse((uint8_t *)p, total, p_m_parser);
	printf("ret1 = %d\n", ret1);

	if (ret1 == 3) {
		printf("parser over\n");
	} else {
		printf("parser error\n");
	}

	printf("p_m_parser->de_len = %d\n", p_m_parser->de_len);

	return 0;
}

