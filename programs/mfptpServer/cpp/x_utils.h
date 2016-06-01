#pragma once
#include "utils.h"

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <assert.h>
#include <openssl/aes.h>
#include <openssl/idea.h>
#include <openssl/objects.h>
#include <openssl/evp.h>

unsigned int mfptp_log(char *imei, char *buf, int len, int up);

/*************************************************/

#define AO_UNLOCK           0
#define AO_INLOCK           1

#define X_DATA_NO_ALL           2
#define X_DATA_IS_ALL           1

#define X_DONE_OK               0
#define X_IO_ERROR              -1
#define X_DATA_TOO_LARGE        -2
#define X_MALLOC_FAILED         -3
#define X_PARSE_ERROR           -4
#define X_INTERIOR_ERROR        -5
#define X_REQUEST_ERROR         -6
#define X_EXECUTE_ERROR         -7
#define X_REQUEST_QUIT          -8
#define X_KV_TOO_MUCH           -9
#define X_NAME_TOO_LONG         -10

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

#define X_LOG_D_VALUE           "[DEBUG]"
#define X_LOG_I_VALUE           "[INFO ]"
#define X_LOG_W_VALUE           "[WARN ]"
#define X_LOG_F_VALUE           "[FAIL ]"
#define X_LOG_E_VALUE           "[ERROR]"
#define X_LOG_S_VALUE           "[SYST ]"
#define X_LOG_M_VALUE           "[MODULE ]"

#define X_LOG_D_LEVEL           0
#define X_LOG_I_LEVEL           1
#define X_LOG_W_LEVEL           2
#define X_LOG_F_LEVEL           3
#define X_LOG_E_LEVEL           4
#define X_LOG_S_LEVEL           5
#define X_LOG_M_LEVEL           6	// add

#define X_LOG_D_COLOR           COLOR_LIGHT_GREEN
#define X_LOG_I_COLOR           COLOR_LIGHT_GREEN
#define X_LOG_W_COLOR           COLOR_LIGHT_YELLOW
#define X_LOG_F_COLOR           COLOR_LIGHT_RED
#define X_LOG_E_COLOR           COLOR_LIGHT_RED
#define X_LOG_S_COLOR           COLOR_LIGHT_RED
#define X_LOG_M_COLOR           COLOR_LIGHT_RED
//////////////////////////////////////////////////////////////////////////////////////////////////
/*  增加模块划分宏  */
#define LOG_JSON_CONF           0x001
#define LOG_INIT                0x002
#define LOG_NET_CONN            0x004
#define LOG_MFPTP_AUTH          0x008

#define LOG_MFPTP_PARSE         0x010

#define LOG_NET_UPLINK          0x020
#define LOG_MFPTP_PACK          0x040
#define LOG_NET_DOWNLINK        0x080
#define LOG_TO_REDIS            0x100
#define LOG_TIMER               0x200

#define LOG_ALL                 0x0ff
#define LOG_NONE                0x000

/*日志文件结构以及全局变量定义*/
#define MIN_LOGFD_RECORD_PEAK   4
typedef struct file_log_s
{
	int             level;
	int             release_log;
	unsigned int    module;	// add by liujinyang
	int             nowfd;
	unsigned int    index;
	int             logfd[MIN_LOGFD_RECORD_PEAK];
	char            path[MAX_FILE_PATH_SIZE];
	char            name[MAX_FILE_NAME_SIZE];

	char            *log_path;
	char            *log_filename;

	int             change_log;
} file_log_t;

// file_log_t g_file_log = {};
// 函数声明

void init_log_output(char *path, char *name, int module, int level, int release_log);

void open_new_log_output(void);

void log_output(int module, int level, const char *value, const char *file, const char *func, unsigned int line, const char *fmt, ...);

void start_change_log();

void log_start();

#define LOG(module, lgt, fmt, args ...) log_output(module, X_LOG_##lgt##_LEVEL, X_LOG_##lgt##_VALUE, __FILE__, __FUNCTION__, __LINE__, fmt, ##args)

/////////////////////////////////////////////////////////////////////////////////////////////////
void init_log(char *path, char *name, int level);

void open_new_log(void);

void dyn_log(int level, const char *value, const char *fmt, ...);

long long get_system_time(void);

/* 名    称: mfptp_bkdr_hash
 * 功    能: 计算字符串的哈西值
 * 参    数: str,输入字符串
 * 返 回 值:  哈西值
 * 修    改: 新生成函数l00167671 at 2015/2/28
 */
unsigned int mfptp_bkdr_hash(char *str);

/* 名    称: mfptp_aes_ecb_decrypt
 * 功    能: aes解密
 * 参    数: key, 解密密钥
 *           crypted, 经过密钥key加密过的数据
 *           decrypted,解密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示解密成功，1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_aes_ecb_decrypt(char *key, char *crypted, char *decrypted, int len);

/* 名    称: mfptp_aes_ecb_encrypt
 * 功    能: aes加密
 * 参    数: key, 加密密钥
 *           data, 明文
 *           decrypted,加密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示加密成功，1表示加密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_aes_ecb_encrypt(char *key, char *data, char *crypted, int len);

/* 名    称: mfptp_ungzip
 * 功    能: 解压缩gz数据
 * 参    数: gz_buf,压缩过的数据
 *           gzlen, 压缩过数据的长度
 *           ungz_buf,解压缩后的数据存储在该指针处
 * 返 回 值: 大于零表示解压缩数据的长度,小于零表示解压缩失败
 * 修    改: 新生成函数l00167671 at 2015/5/23
 */
int mfptp_ungzip(char *gz_buf, int gzlen, char *ungz_buf, int ungz_buf_len);

/* 名    称: mfptp_idea_ecb_decrypt
 * 功    能: idea解密
 * 参    数: key, 解密密钥
 *           in , 密文
 *           out, 解密后的明文
 *           len, 加密和解密的数据长度
 * 返 回 值: 0,表示解密成功，1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_idea_ecb_decrypt(char *key, char *in, char *out, int len);

/* 名    称: mfptp_idea_ecb_encrypt
 * 功    能: idea加密
 * 参    数: key, 加密密钥
 *           in , 明文
 *           out, 加密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示加密成功，1表示加密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_idea_ecb_encrypt(unsigned char *key, unsigned char *in, unsigned char *out, int len);

/* 名    称: mfptp_unzip
 * 功    能: 解压缩gz数据
 * 参    数: gz_buf,压缩过的数据
 *           gzlen, 压缩过数据的长度
 *           ungz_buf,解压缩后的数据存储在该指针处
 * 返 回 值: 大于零表示解压缩数据的长度,小于零表示解压缩失败
 * 修    改: 新生成函数l00167671 at 2015/5/23
 */
int mfptp_unzip(char *gz_buf, int gzlen, char *ungz_buf, int ungz_buf_len);

void mfptp_set_user_secret_key(char key[16]);

