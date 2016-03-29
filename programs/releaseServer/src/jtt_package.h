#ifndef __JTT_PACKAGE_H__
#define __JTT_PACKAGE_H__

#include "pub_incl.h"

#define HEAD_FLAG       '['
#define END_FLAG        ']'
#define VERSION(ptr) ptr[0] = 0x01; ptr[1] = 0x02; ptr[2] = 0x0F

typedef struct _PKG_BODY
{
	unsigned        msg_id;	/*数据体业务类型*/
	void            *body;	/*数据体指针*/
	unsigned        blen;	/*数据体大小*/
} PKG_BODY;

typedef struct _PKG_USER
{
	char            encrypt;	// 与对端通讯数据体是否加密
	unsigned        msg_enterid;	//下级平台接入码，有对端系统分配给本平台

	/*私有全程量*/
	unsigned        msg_sz;		// 此字段，非配置值，而是套接字相关的包的序列号,每发生一个包其值加一

	/*常量*/
	char            head_flag;	// 头标志[
	char            end_flag;	// 尾标志]
	char            version[3];	// 协议版本号
} PKG_USER;

#pragma pack(push,1)

typedef struct _JTT_HEAD
{
	char            head_flag;	// 头标志
	unsigned int    msg_length;	// 数据长度:包括头标志，头，体,CRC和尾标志
	unsigned int    msg_sz;		// 报文序列号
	unsigned short  msg_id;		// 业务数据类型
	unsigned int    msg_enterid;	//下级平台接入码
	char            ver[3];		// 版本号
	char            encrypt;	// 加密标志:0-不加密，1-加密
	unsigned int    key;		// 加密密钥
} JTT_HEAD;

#pragma pack(pop)

int pkg_encode(PKG_USER *user, unsigned char *pkg, int *plen, PKG_BODY body);

int pkg_decode(PKG_USER user, unsigned char *pkg, int *pkg_len);
#endif	/* ifndef __JTT_PACKAGE_H__ */

