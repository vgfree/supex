//
//  cache.h
//  supex
//
//  Created by 周凯 on 15/12/24.
//  Copyright © 2015年 zhoukai. All rights reserved.
//

#ifndef cache_h
#define cache_h

#include <libmini.h>
#include "../base/utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 读写同时进行无需加锁，读写过程中有移动/扩展操作需要加锁,可以通过链表cache实现分段发送，每递交一个cache都是完结的//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
__BEGIN_DECLS

/* ----------- 参数 ----------- */

#ifndef CACHE_MAXSIZE
  #define CACHE_MAXSIZE (1024 * 1024 * 10)
#endif

#ifndef CACHE_INITSIZE
  #define CACHE_INITSIZE 128
#endif

#ifndef CACHE_INCSIZE
  #define CACHE_INCSIZE 64
#endif

/* ----------- 检验参数是否正确 ----------- */

#if CACHE_MAXSIZE < 1
  #error "ERROR PARAMRTER `CACHE_MAXSIZE`"
#endif

#if CACHE_INITSIZE <1 || CACHE_INITSIZE> CACHE_MAXSIZE
  #error "ERROR PARAMRTER `CACHE_INITSIZE`"
#endif

#if CACHE_INCSIZE <1 || CACHE_INCSIZE> CACHE_MAXSIZE
  #error "ERROR PARAMRTER `CACHE_INCSIZE`"
#endif
/* ------------------------------------- */
struct cache
{
	/* ----- private : only read ------     */
	int             magic;
	AO_SpinLockT    lock;		/**< 分段锁 */
	/* ----- private : write & read ------  */
	unsigned        maxsize;	/**<最大容量*/
	unsigned        initsize;	/**<初始化大小*/
	unsigned        incsize;	/**<每次增长的大小*/
	/* ----- private : only read ------     */
	unsigned        start;		/**<有效数据起始下标*/
	unsigned        end;		/**<有效数据结束下标*/
	unsigned        size;		/**<缓冲长度*/
	char            *buff;		/**<缓冲地址*/
	/* ----- public : write & read ------   */
	char            *usr;		/**<用户层附加数据指针*/
};

/*get length of data and first pointer of data*/
#define cache_data_length(cptr)         ((cptr)->end - (cptr)->start)
#define cache_data_address(cptr)        (&(cptr)->buff[(cptr)->start])

void cache_peak(unsigned int one_pack_max_size, unsigned int add_read_max_size);

/**
 * 初始化CACHE
 */
bool cache_initial(struct cache *cache);

void cache_finally(struct cache *cache);

/**移动内部的空闲空间*/
void cache_movemem(struct cache *cache);

/**收缩内部空闲空间*/
void cache_cutmem(struct cache *cache);

/**清除所有数据，并收缩空间*/
void cache_clean(struct cache *cache);

/**
 * 增加空间
 *@param size<0时，使用默认值，＝0时，返回剩余空间
 *@return -1失败，设置错误值到errno中；>0可用空间大小，可能小于期望增长值
 */
ssize_t cache_incrmem(struct cache *cache, int size);

/**
 *按顺序取出cache中的数据到data中
 *@return 返回-1，失败，并设置错误值到errno中；返回取出到数据量
 */
ssize_t cache_get(struct cache *cache, char *data, unsigned size);

/**
 *将data中的数据追加到cache的尾部
 *@param size，-1时，将data视为c字符串
 *@return 返回-1，失败，并设置错误值到errno中；返回放入的数据量
 */
ssize_t cache_append(struct cache *cache, const char *data, int size);

/**
 * 以格式化的方式追加数据，但不将'\0'加入
 * @see cache_append
 */
ssize_t cache_appendf(struct cache *cache, const char *fmt, ...) __printflike(2, 3);

/**
 * 以格式化的方式追加数据
 * @see cache_appendf
 */
ssize_t cache_vappendf(struct cache *cache, const char *fmt, va_list ap);

/**
 *将data中的数据插入到cache的头部
 *@param size，-1时，将data视为c字符串
 *@return 返回-1，失败，并设置错误值到errno中；返回放入的数据量
 */
ssize_t cache_insert(struct cache *cache, const char *data, int size);

/**
 * 以格式化的方式插入数据，但不将'\0'加入
 * @see cache_insert
 */
ssize_t cache_insertf(struct cache *cache, const char *fmt, ...) __printflike(2, 3);

/**
 * 以格式化的方式插入数据
 * @see cache_insertf
 */
ssize_t cache_vinsertf(struct cache *cache, const char *fmt, va_list ap);

ssize_t cache_read(struct cache *cache, int fd, int size);

ssize_t cache_write(struct cache *cache, int fd, int size);

/* --------------		*/
__END_DECLS
#endif	/* cache_h */

