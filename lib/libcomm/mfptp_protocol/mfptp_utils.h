/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __MFPTP_UTILS_H__
#define __MFPTP_UTILS_H__

#include "../comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif



#define	MFPTP_MAJOR_VERSION	1	/* MFPTP协议的主版本号 */
#define MFPTP_MINOR_VERSION	0	/* MFPTP协议的副版本号 */
#define	MFPTP_MAX_FRAMES	13	/* MFPTP协议单包支持最大的帧的数量 */
#define	MFPTP_MAX_PACKAGES	8	/* MFPTP协议支持携带的最大包数 */

#define MFPTP_HEADER_LEN	10	/* MFPTP协议头的所占的字节数 */
#define MFPTP_FP_CONTROL_LEN	1	/* MFPTP协议FP_control的所占的字节数 */
#define MFPTP_F_SIZE_MINLEN	1	/* MFPTP协议F_size所占的最小字节数 */
#define MFPTP_F_SIZE_MAXLEN	4	/* MFPTP协议F_size所占的最大字节数 */


/* MFPTP协议压缩格式 */
enum mfptp_compression_type {
	NO_COMPRESSION	= 0x00 << 4,
	ZIP_COMPRESSION	= 0x01 << 4,
	GZIP_COMPRESSION= 0x02 << 4
};

/* MFPTP协议加密格式 */
enum mfptp_encryption_type {
	NO_ENCRYPTION = 0x0,
	IDEA_ENCRYPTION,
	AES_ENCRYPTION
};

/* MFPTP协议的socket的type */
enum mfptp_socket_type {
	PAIR_METHOD = 0x00,
	PUB_METHOD,
	SUB_METHOD,
	REQ_METHOD,
	REP_METHOD,
	DEALER_METHOD,
	ROUTER_METHOD,
	PULL_METHOD,
	PUSH_METHOD,
	HEARTBEAT_METHOD,
	INVALID_METHOD
};

enum mfptp_error {
	HEADER_LEN_INVAILD = 0x00;	/* MFPTP包头长度不合法 */
};

/* MFPTP协议帧的相关信息[一个此结构体代表的是一个包的数据] */
typedef struct mfptp_frame_info {
	int frames;				/* 一共有多少帧 */
	int frame_size[MFPTP_MAX_FRAMES];	/* 每一帧的数据大小 */
	int frame_offset[MFPTP_MAX_FRAMES];	/* 每帧的偏移 */
};

/* MFPTP协议包的相关信息 */
struct mfptp_package_info {
	int			packages;			/* 总包数 */
	int			frames;				/* 总帧数 */
	int			dsize;				/* 数据总大小 */
	struct mfptp_frame_info frame[MFPTP_MAX_PACKAGES];	/* 包里面帧的相关信息 */
};

/* MFPTP协议的包头相关信息 */
struct mfptp_header_info {
	int packages;				/* 包的数量 */
	int encryption;				/* 加密格式 */
	int compression;			/* 压缩格式 */
	int socket_type;			/* socket的类型 */
	int major_version;			/* 主版本号 */
	int minor_version;			/* 副版本号*/
};

     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __MFPTP_UTILS_H__ */
