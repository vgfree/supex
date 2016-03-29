//
//  node_manager.h
//  节点管理器，所有插入节点被散列到多个跳表中
//
//  Created by 周凯 on 14/07.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#ifndef __malloc_hook_node_manager_h__
#define __malloc_hook_node_manager_h__

#include "libmini.h"

__BEGIN_DECLS

extern int gmhookTraceLogFD;

/*
 * 跳表最大层数
 * 跳表节点数量到达一定值时，层数越多越耗内存，但操作效率有很大的提高
 */
#define MANAGER_MAX_LAYER       (32)

/*
 * 哈希表桶数（质数），大于1亿个节点时请修改为3位数（以上）的质数
 * 多线程操作时，哈希桶越大可以降低竞争
 */
#define MANAGER_MAX_BUCKETS     (301)

/*
 * 要增加跳表每个节点的层数，请增加此宏，必须小于1，且为浮点型
 */
#define MANAGER_RAND_SEED       (.5F)

/* ------------------------                   */

/**
 * 增加一个动态内存指针
 */
bool PushManager(void *ptr, long nbytes, void *btinfo);

/**
 * 删除一个动态内存指针
 */
bool PopManager(void *ptr, long *nbytes, void *btinfo);

/**
 * 查找一个动态内存指针
 */
bool PullManager(void *ptr, long *nbytes);

/**
 * 内存管理器中的内存块数量
 */
long ManagerTotal();

/**
 * 打印内存管理器中的节点信息
 */
void ManagerPrint(const char *file, int fd);

__END_DECLS
#endif	/* ifndef __malloc_hook_node_manager_h__ */

