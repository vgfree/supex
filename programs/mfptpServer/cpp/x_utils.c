#define _GNU_SOURCE	/* See feature_test_macros(7) */
#include <fcntl.h>	/* Obtain O_* constant definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <mqueue.h>
#include <stdarg.h>
#include "x_utils.h"
#include <zlib.h>
#include <openssl/rand.h>
/*********************************TIME***********************************************/

long long get_system_time(void)
{
	struct timeb t;

	ftime(&t);
	return 1000 * t.time + t.millitm;
}

#if 0
  #include <time.h>
static inline unsigned long long _get_time_cycle(void)
{
	__asm("RDTSC");
}

static int get_array_random_index(int max)	// not effect
{
	long long count = _get_time_cycle();

	return count % max;
}
#endif

/*********************************LOGS***********************************************/
#define MIN_LOGFD_RECORD_PEAK 4	// should >= 4 or max pthread counts call 'open_new_log'
struct log_file
{
	int             level;
	int             nowfd;
	unsigned int    index;
	int             logfd[MIN_LOGFD_RECORD_PEAK];
	char            path[MAX_FILE_PATH_SIZE];
	char            name[MAX_FILE_NAME_SIZE];
};

static struct log_file          g_log_file = {};
static struct safe_once_init    g_init_log_mark = { 0, NULLOBJ_AO_SPINLOCK };
static struct safe_once_init    g_init_log_file_mark = { 0, NULLOBJ_AO_SPINLOCK };	// 调试log

void init_log(char *path, char *name, int level)
{
	//	printf("x_printf:path %s name %s,level :%d\n",path,name,level);
	SAFE_ONCE_INIT_COME(&g_init_log_mark);

	memset(&g_log_file, 0, sizeof(struct log_file));
	g_log_file.level = level;
	strncat(g_log_file.path, path, MAX_FILE_PATH_SIZE - 1);
	strncat(g_log_file.name, name, MAX_FILE_NAME_SIZE - 1);

	SAFE_ONCE_INIT_OVER(&g_init_log_mark);
}

void open_new_log(void)
{
	//	printf("x_printf: path = %s,name = %s\n",g_log_file.path,g_log_file.name);
	int             newfd, oldfd = 0;
	time_t          t_now = time(NULL);
	struct tm       *p_tm = localtime(&t_now);
	char            name[64] = { 0 };

	snprintf(name, (sizeof(name) - 1), "%s%s_%04d%02d.log", g_log_file.path, g_log_file.name, 1900 + p_tm->tm_year, 1 + p_tm->tm_mon);
	newfd = open(name, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	//	printf("x_printf:file fd = %d\n",newfd);

	int offset = __sync_fetch_and_add(&g_log_file.index, 1) % MIN_LOGFD_RECORD_PEAK;
	do {
		oldfd = g_log_file.logfd[offset];

		if (oldfd > 0) {
			close(oldfd);
		}

		__sync_lock_test_and_set(&g_log_file.nowfd, newfd);
	} while (!__sync_bool_compare_and_swap(&g_log_file.logfd[offset], oldfd, newfd));
}

void dyn_log(int level, const char *value, const char *fmt, ...)
{
	if (level < g_log_file.level) {
		return;	/* too verbose */
	}

	/* get current time and log level */
	time_t  now = time(NULL);
	char    time_buf[33] = { 0 };
	strftime(time_buf, sizeof(time_buf) - 1, "%y%m%d_%H%M%S", localtime(&now));
	dprintf(g_log_file.nowfd, "%s %s%16s(%20s)|%4d|-->", time_buf, value, __FILE__, __FUNCTION__, __LINE__);
	/*write mesage*/
	va_list ap;
	va_start(ap, fmt);
	vdprintf(g_log_file.nowfd, fmt, ap);
	va_end(ap);
	/* write to log and flush to disk. */
}

/****************************************************************************************************************
 *                                     new log system
 * *************************************************************************************************************/
#if 1
file_log_t g_file_log = {};

/*
 *功 能：根据json文件中获取的日志路径，日志文件名，初始日志等级来初始化日志系统
 *参 数: path --- 日志路径
 *         name --- 日志名称
 *                 level ---日志等级
 *                 module ---日志模块
 *                 release_log---上线需要关闭release log
 *返回值：void
 *
 **/
void init_log_output(char *path, char *name, int module, int level, int release_log)
{
	SAFE_ONCE_INIT_COME(&g_init_log_file_mark);

	memset(&g_file_log, 0, sizeof(struct file_log_s));
	g_file_log.level = level;
	g_file_log.module = module;
	g_file_log.release_log = release_log;
	strncat(g_file_log.path, path, MAX_FILE_PATH_SIZE - 1);
	strncat(g_file_log.name, name, MAX_FILE_NAME_SIZE - 1);

	SAFE_ONCE_INIT_OVER(&g_init_log_file_mark);
}

void open_new_log_output(void)
{
	// printf("LOG: path = %s,name = %s\n",g_file_log.path,g_file_log.name);
	time_t          t_now = time(NULL);
	struct tm       *p_tm = localtime(&t_now);
	char            name[64] = { 0 };

	snprintf(name, (sizeof(name) - 1), "%s%s_%04d%02d.log.debug", g_file_log.path, g_file_log.name, 1900 + p_tm->tm_year, 1 + p_tm->tm_mon);
	g_file_log.nowfd = open(name, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);

	if (g_file_log.nowfd < 0) {
		printf("LOG:文件创建失败,errno: %s\n", strerror(errno));
	}

	// else
	//	printf("LOG:file fd = %d\n",g_file_log.nowfd);
}

/*
 *功 能：根据打印模块和打印级别来开启打印
 *参 数：module---打印模块
 *         level ---打印级别
 *修 改：modify by 刘金阳 ,增加打印模块参数(module)
 **/
void log_output(int module, int level, const char *value, const char *file, const char *func, unsigned int line, const char *fmt, ...)
{
  #if 0
	printf("enter log output!,fd = %d\n", g_file_log.nowfd);
	printf("release_log = %d\n", g_file_log.release_log);
	printf("module = 0x%08x,g_file_log.module = 0x%08x, level = %d,g_file_log.level = %d\n", module, g_file_log.module, level, g_file_log.level);
  #endif

	// release则关闭调试打印系统
	if (g_file_log.release_log == 1) {
		return;
	}

	if (module & g_file_log.module) {
		if (level >= g_file_log.level) {
			/* get current time and log level */
			time_t  now = time(NULL);
			char    time_buf[33] = { 0 };
			strftime(time_buf, (sizeof(time_buf) - 1), "%y%m%d_%H%M%S", localtime(&now));
			dprintf(g_file_log.nowfd, "%s %s%16s(%20s)|%4d|-->", time_buf, value, file, func, line);// 需要打印出函数所在的行
			/*write mesage*/
			va_list ap;
			va_start(ap, fmt);
			vdprintf(g_file_log.nowfd, fmt, ap);
			va_end(ap);
			/* write to log and flush to disk. */
		}
	}
}

/*
 *功 能：日志系统初始化封装
 *参 数：无
 *返回值：无
 **/
void log_start()
{
	const char *log_conf_filename = "log_conf.json";

	// 信号发生时，需要再次加载日志配置文件,这里是初始化的时候调用
	g_file_log.module = 0;
	load_log_cfg_file(&g_file_log, log_conf_filename);
	// printf("%s:%d release_log = %d\n",__func__,__LINE__,g_file_log.release_log);

	char    *log_path = g_file_log.log_path;
	char    *log_filename = g_file_log.log_filename;
	int     init_log_level = g_file_log.level;
	int     module = g_file_log.module;
	int     release_log = g_file_log.release_log;
	// 日志系统初始化
	init_log_output(log_path, log_filename, module, init_log_level, release_log);
	open_new_log_output();
}

void start_change_log()
{
	while (1) {
		if (g_file_log.change_log == 1) {
			// 加载配置文件
			log_start();
			printf("log configure file success!\n");
			// reset
			g_file_log.change_log = 0;
		}

		sleep(5);
	}
}
#endif	/* if 1 */

/* 名    称: mfptp_bkdr_hash
 * 功    能: 计算字符串的哈西值
 * 参    数: str,输入字符串
 * 返 回 值:  哈西值
 * 修    改: 新生成函数l00167671 at 2015/2/28
 */
unsigned int mfptp_bkdr_hash(char *str)
{
	unsigned int    seed = 131;	// 31 131 1313 13131 131313 etc..
	unsigned int    hash = 0;

	while (*str) {
		hash = hash * seed + (*str++);
	}

	return hash & 0x7FFFFFFF;
}

/* 名    称: mfptp_bkdr_hash
 * 功    能: 计算字符串的哈西值
 * 参    数: str,输入字符串
 * 返 回 值:  哈西值
 * 修    改: 新生成函数l00167671 at 2015/2/28
 */
unsigned int mfptp_log(char *imei, char *buf, int len, int up)
{
	FILE            *fp;
	static int      i = 0;
	char            hdr[1024];
	char            filename[256];

	time_t start = time(NULL);

	sprintf(hdr, "%6d   %lu %s-%lu%d %d\n", i, start, imei, start, i, len);

	if (1 == up) {
		sprintf(filename, "/data/up/%s-%lu-%d", imei, start, i);
	} else {
		sprintf(filename, "/data/down/%s-%lu-%d", imei, start, i);
	}

	fp = fopen("all.log", "a+");

	if (NULL == fp) {
		fprintf(stderr, "致命问题，日志打开失败\n");
	} else {
		int ret = fprintf(fp, hdr, strlen(hdr));

		if (ret < 0) {
			fprintf(stderr, "致命问题，写日志失败\n");
		}

		fclose(fp);
	}

	fp = fopen(filename, "w+");

	if (NULL == fp) {
		fprintf(stderr, "致命问题，数据记录文件打开失败\n");
	} else {
		int ret = fwrite(buf, len, 1, fp);

		if (ret < 0) {
			fprintf(stderr, "致命问题，写数据记录文件失败\n");
		}

		fclose(fp);
	}

	i++;
	return 0;
}

/* 名    称: mfptp_aes_ecb_decrypt
 * 功    能: aes解密
 * 参    数: key, 解密密钥
 *           crypted, 经过密钥key加密过的数据
 *           decrypted,解密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示解密成功，1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_aes_ecb_decrypt(char *key, char *crypted, char *decrypted, int len)
{
	int             i = 0;
	unsigned char   out[AES_BLOCK_SIZE];
	AES_KEY         aes_key;

	if (len % AES_BLOCK_SIZE != 0) {
		len = (len / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	} else {}

	for (i = 0; i < 16; ++i) {
		key[i] = i;
	}

	if (AES_set_decrypt_key(key, 128, &aes_key) < 0) {
		fprintf(stderr, "Unable to set encryption key in AES\n");
		exit(-1);
	}

	for (i = 0; i < len / AES_BLOCK_SIZE; i++) {
		AES_ecb_encrypt(&crypted[i * AES_BLOCK_SIZE], out, &aes_key, AES_DECRYPT);
		memcpy(&decrypted[i * AES_BLOCK_SIZE], out, AES_BLOCK_SIZE);
	}
}

/* 名    称: mfptp_aes_ecb_encrypt
 * 功    能: aes加密
 * 参    数: key, 加密密钥
 *           data, 明文
 *           decrypted,加密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示加密成功，1表示加密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_aes_ecb_encrypt(char *key, char *data, char *crypted, int len)
{
	/*
	 *    int           i = 0;
	 *    AES_KEY       aes_key;
	 *
	 *    if( len%AES_BLOCK_SIZE != 0){
	 *        len = (len/AES_BLOCK_SIZE+1)*AES_BLOCK_SIZE;
	 *    }
	 *
	 *    for ( i=0; i<16; ++i) {
	 *        key[i] =  i;
	 *    }
	 *
	 *    if (AES_set_decrypt_key(key, 128, &aes_key) < 0) {
	 *        fprintf(stderr, "Unable to set encryption key in AES\n");
	 *        exit(-1);
	 *    }
	 *
	 *    for(i = 0; i < len/AES_BLOCK_SIZE; i++){
	 *        unsigned char out[AES_BLOCK_SIZE];
	 *        AES_ecb_encrypt(&crypted[i*AES_BLOCK_SIZE], out, &aes_key, AES_ENCRYPT);
	 *        memcpy(&decrypted[i*AES_BLOCK_SIZE], out, AES_BLOCK_SIZE);
	 *    }
	 */
	return 0;
}

#define IDEA_BLOCK_SIZE (8)

/* 名    称: mfptp_idea_ecb_encrypt
 * 功    能: idea加密
 * 参    数: key, 加密密钥
 *           in , 明文
 *           out, 加密后的数据存储在该地址指向的内存中
 * 返 回 值: 0,表示加密成功，1表示加密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_idea_ecb_encrypt(unsigned char *key, unsigned char *in, unsigned char *out, int len)
{
	IDEA_KEY_SCHEDULE       idea_key;
	IDEA_KEY_SCHEDULE       idea_dkey;
	unsigned char           out_block[AES_BLOCK_SIZE];
	int                     i = 0;

	if (len % IDEA_BLOCK_SIZE != 0) {
		len = (len / IDEA_BLOCK_SIZE + 1) * IDEA_BLOCK_SIZE;
	} else {}

	idea_set_encrypt_key(key, &idea_key);

	for (i = 0; i < len / IDEA_BLOCK_SIZE; i++) {
		idea_ecb_encrypt(&in[i * IDEA_BLOCK_SIZE], out_block, &idea_key);
		memcpy(&out[i * IDEA_BLOCK_SIZE], out_block, IDEA_BLOCK_SIZE);
	}
}

/* 名    称: mfptp_idea_ecb_decrypt
 * 功    能: idea解密
 * 参    数: key, 解密密钥
 *           in , 密文
 *           out, 解密后的明文
 *           len, 加密和解密的数据长度
 * 返 回 值: 0,表示解密成功，1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_idea_ecb_decrypt(char *key, char *in, char *out, int len)
{
	IDEA_KEY_SCHEDULE       idea_key;
	IDEA_KEY_SCHEDULE       idea_dkey;
	unsigned char           out_block[IDEA_BLOCK_SIZE];
	int                     i = 0;

	if (len % IDEA_BLOCK_SIZE != 0) {
		len = (len / IDEA_BLOCK_SIZE + 1) * IDEA_BLOCK_SIZE;
	} else {}

	idea_set_encrypt_key(key, &idea_key);
	idea_set_decrypt_key(&idea_key, &idea_dkey);

	for (i = 0; i < len / IDEA_BLOCK_SIZE; i++) {
		idea_ecb_encrypt(&in[i * IDEA_BLOCK_SIZE], out_block, &idea_dkey);
		memcpy(&out[i * IDEA_BLOCK_SIZE], out_block, IDEA_BLOCK_SIZE);
	}
}

/* 名    称: mfptp_ungzip
 * 功    能: 解压缩gz数据
 * 参    数: gz_buf,压缩过的数据
 *           gzlen, 压缩过数据的长度
 *           ungz_buf,解压缩后的数据存储在该指针处
 * 返 回 值: 大于零表示解压缩数据的长度,小于零表示解压缩失败
 * 修    改: 新生成函数l00167671 at 2015/5/23
 */
int mfptp_ungzip(char *gz_buf, int gzlen, char *ungz_buf, int ungz_buf_len)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != inflateInit2(&s, MAX_WBITS + 16)) {
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

/* 名    称: mfptp_unzip
 * 功    能: 解压缩gz数据
 * 参    数: gz_buf,压缩过的数据
 *           gzlen, 压缩过数据的长度
 *           ungz_buf,解压缩后的数据存储在该指针处
 * 返 回 值: 大于零表示解压缩数据的长度,小于零表示解压缩失败
 * 修    改: 新生成函数l00167671 at 2015/5/23
 */
int mfptp_unzip(char *z_buf, int zlen, char *unz_buf, int unz_buf_len)
{
	z_stream s;

	s.zalloc = Z_NULL;
	s.zfree = Z_NULL;
	s.opaque = Z_NULL;

	if (Z_OK != inflateInit(&s)) {
		return -1;
	}

	s.next_in = z_buf;
	s.avail_in = zlen;
	s.next_out = unz_buf;
	s.avail_out = unz_buf_len;

	if (Z_STREAM_END != inflate(&s, Z_FINISH)) {
		return -1;
	}

	if (Z_OK != inflateEnd(&s)) {
		return -1;
	}

	return s.total_out;
}

/* 名    称: mfptp_aes_ecb_decrypt_evp
 * 功    能: aes解密
 * 参    数: key, 解密密钥
 *           crypted, 经过密钥key加密过的数据
 *           decrypted,解密后的数据存储在该地址指向的内存中
 * 返 回 值: 大于等于零,表示解密长度，-1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
int mfptp_aes_ecb_decrypt_evp(char *key, int key_len, char *crypted, char *decrypted, int len)
{
	int             de_len1;
	int             de_len2;
	EVP_CIPHER_CTX  ctx;

	EVP_CIPHER_CTX_init(&ctx);
	int     ret = EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char *)key, NULL);
	int     rv = EVP_DecryptUpdate(&ctx, decrypted, &de_len1, crypted, len);

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

/* 名    称: mfptp_idea_ecb_decrypt_evp
 * 功    能: idea解密
 * 参    数: key, 解密密钥
 *           key_len,密钥长度
 *           crypted, 经过密钥key加密过的数据
 *           decrypted,解密后的数据存储在该地址指向的内存中
 *           len, 密文长度
 * 返 回 值: 大于等于零,表示解密长度，-1表示解密失败
 * 修    改: 新生成函数l00167671 at 2015/5/20
 */
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

	rv = EVP_DecryptFinal_ex(&ctx, decrypted + de_len1, &de_len2);

	if (rv != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	EVP_CIPHER_CTX_cleanup(&ctx);

	return de_len1 + de_len2;
}

/* 名    称: mfptp_set_user_secret_key
 * 功    能: 生成用户的加密密钥
 * 参    数: key,存储密钥,长度16字节128bit
 * 返 回 值: 无
 * 修    改: 新生成函数l00167671 at 2015/5/27
 */
void mfptp_set_user_secret_key(char key[16])
{
	if (1 == RAND_bytes(key, 16)) {} else {
		int i = 0;

		for (; i < 16; i++) {
			key[i] = rand() % 255;
		}
	}
}

