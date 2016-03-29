#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>		/* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <assert.h>
#include <dirent.h>
#include <sys/uio.h>
#include <errno.h>

#define MAX_FILE_NAME_SIZE      64
#define MAX_TRANSIT_SIZE        (1024 * 4)
#define MAX_ABSOLUTE_PATH       1024
#define USLEEP_TIME_APART       1000

// leveldb 参数设置
#define HIT_LDB_NAME            "loghitDB"
#define HIT_LDB_BLK_SIZE        (32 * 1024)
#define HIT_LDB_WB_SIZE         (64 * 1024 * 1024)
#define HIT_LDB_LRU_SIZE        (64 * 1024 * 1024)
#define HIT_LDB_BLOOM_SIZE      (10)

/*
 * 终端色彩定义
 */
#define COLOR_NONE              "\x1B[m"
#define COLOR_GRAY              "\x1B[0;30m"
#define COLOR_LIGHT_GRAY        "\x1B[1;30m"
#define COLOR_RED               "\x1B[0;31m"
#define COLOR_LIGHT_RED         "\x1B[1;31m"
#define COLOR_GREEN             "\x1B[0;32m"
#define COLOR_LIGHT_GREEN       "\x1B[1;32m"
#define COLOR_YELLOW            "\x1B[0;33m"
#define COLOR_LIGHT_YELLOW      "\x1B[1;33m"
#define COLOR_BLUE              "\x1B[0;34m"
#define COLOR_LIGHT_BLUE        "\x1B[1;34m"
#define COLOR_PURPLE            "\x1B[0;35m"
#define COLOR_LIGHT_PURPLE      "\x1B[1;35m"
#define COLOR_CYAN              "\x1B[0;36m"
#define COLOR_LIGHT_CYAN        "\x1B[1;36m"
#define COLOR_WHITE             "\x1B[0;37m"
#define COLOR_LIGHT_WHITE       "\x1B[1;37m"

#define LOG_D_VALUE             "[DEBUG]"
#define LOG_I_VALUE             "[INFO ]"
#define LOG_W_VALUE             "[WARN ]"
#define LOG_F_VALUE             "[FAIL ]"
#define LOG_E_VALUE             "[ERROR]"
#define LOG_S_VALUE             "[SYST ]"

#define LOG_D_COLOR             COLOR_LIGHT_GREEN
#define LOG_I_COLOR             COLOR_LIGHT_GREEN
#define LOG_W_COLOR             COLOR_LIGHT_YELLOW
#define LOG_F_COLOR             COLOR_LIGHT_RED
#define LOG_E_COLOR             COLOR_LIGHT_RED
#define LOG_S_COLOR             COLOR_LIGHT_RED

#ifdef DEBUG
  #define x_printf(lgt, fmt, ...)								\
	fprintf(stdout, LOG_##lgt##_COLOR LOG_##lgt##_VALUE					\
		COLOR_LIGHT_CYAN "%16s" COLOR_LIGHT_PURPLE "(%20s)" COLOR_LIGHT_BLUE "|%4d|-->"	\
		LOG_##lgt##_COLOR fmt COLOR_NONE, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
  #define x_printf(lgt, fmt, args ...) ;
#endif

// 文件偏移点存储信息
struct store_node
{
	ino_t   sn_ino;		// 索引节点编号
	off_t   sn_size;	// 文件大小
	time_t  sn_atime;	//上次访问时间
	time_t  sn_mtime;	//上次修改时间
	time_t  sn_ctime;	// 状态改变的最后时间
	off_t   sn_done;	// 偏移？
};

