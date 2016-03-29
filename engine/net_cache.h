#pragma once

#include "utils.h"

struct net_cache
{
	unsigned        max_size;			/**< 当前缓冲块的长度*/
	unsigned        get_size;			/**< 当前缓冲增加数据的长度*/
	unsigned        out_size;			/**< 当前缓冲丢弃数据的长度*/
	int             back_size;
#ifdef SECTION_SEND
	int             send_finish;		/**< 当前数据是否发送完成 */
	AO_SpinLockT    lock;			/**< 分段锁 */
#endif
	char            *buf_addr;		/**< 实际缓冲地址*/
	char            base[MAX_DEF_LEN];	/**< 预设缓冲块*/
};

void cache_peak(unsigned int max);

void cache_init(struct net_cache *info);

int cache_add(struct net_cache *info, const char *data, int size);

#ifdef  SECTION_SEND
int cache_add_section(struct net_cache *info, const char *data, int size);
#endif

int cache_set(struct net_cache *info, const char *data, int max, int size);

ssize_t net_recv(struct net_cache *cache, int sockfd, int *status);

ssize_t net_send(struct net_cache *cache, int sockfd, int *status);

void cache_free(struct net_cache *info);

#ifdef _mttptest
void cache_move(struct net_cache *cache);
#endif

ssize_t x_net_recv(struct net_cache *cache, int sockfd, int *status);

