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

/* MFPTP协议错误码 */
enum mfptp_error {
	MFPTP_PARSER_OK = 0x00,		/* MFPTP解析的时候没发生错误，进行下一步的解析 */
	MFPTP_PARSER_OVER,		/* MFPTP解析成功的完成 */
	MFPTP_HEAD_INVAILD,		/* MFPTP协议解析前6个字节“#MFPTP”出错 */

};

/* MFPTP协议的状态码 */
enum mfptp_status {
	MFPTP_PARSE_INIT = 0x00,/* MFPTP协议解析初始化的状态 */
	MFPTP_PACKAGE_INIT,	/* MFPTP协议打包初始化的状态 */
	MFPTP_HEAD,		/* MFPTP协议的前6个字节"#MFPTP" */
	MFPTP_VERSION,		/* MFPTP协议的版本号 */
	MFPTP_CONFIG,		/* MFPTP协议的压缩解密格式 */
	MFPTP_SOCKET_TYPE,	/* MFPTP协议socket的类型 */
	MFPTP_PACKAGES,		/* MFPTP协议携带的包数 */
	MFPTP_FP_CONTROL,	/* MFPTP协议的FP_control字段 */
	MFPTP_F_SIZE,		/* MFPTP协议F_size字段 */
	MFPTP_FRAME,		/* MFPTP协议的帧 */
	MFPTP_FRAME_OVER,	/* MFPTP协议解析完一包的数据 */
	MFPTP_PACKAGE_OVER	/* MFPTP协议解析完 */
};

/* MFPTP协议帧的相关信息[一个此结构体代表的是一个包的数据] */
struct mfptp_frame_info {
	int frames;				/* 一共有多少帧 */
	int frame_size[MFPTP_MAX_FRAMES];	/* 每一帧的数据大小 */
	int frame_offset[MFPTP_MAX_FRAMES];	/* 每帧的偏移 */
};

/* MFPTP协议包的相关信息 */
struct mfptp_package_info {
	bool			init;
	int			packages;			/* 总包数 */
	int			frames;				/* 总帧数 */
	int			dsize;				/* 数据总大小 */
	struct mfptp_frame_info frame[MFPTP_MAX_PACKAGES];	/* 包里面帧的相关信息 */
};

/* MFPTP协议的包头相关信息 */
struct mfptp_header_info {
	int		packages;			/* 包的数量 */
	int		f_size;				/* F_size的字段 */
	unsigned char	not_end;			/* 当前帧是否是最后一帧 */
	unsigned char	size_f_size;			/* F_size字段所占字节数 */
	unsigned char	encryption;			/* 加密格式 */
	unsigned char	compression;			/* 压缩格式 */
	unsigned char	socket_type;			/* socket的类型 */
	unsigned char	major_version;			/* 主版本号 */
	unsigned char	minor_version;			/* 副版本号*/
};

     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __MFPTP_UTILS_H__ */
