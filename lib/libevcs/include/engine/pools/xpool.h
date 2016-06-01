#pragma once

#include "../base/utils.h"
#include "../base/free_queue.h"

#define POOL_API_OK             0
#define POOL_API_ERR_NO_POOL    -1
#define POOL_API_ERR_IS_FULL    -2
#define POOL_API_ERR_OP_FAIL    -3

struct xpool;
typedef int (*POOL_ACTION_OPT)(struct xpool *pool, void **cite, va_list *ap);

struct xpool
{
	int                     max;
	int                     use;				/*已使用数量*/
	char                    tag[128];
	POOL_ACTION_OPT         cso_open;
	POOL_ACTION_OPT         cso_exit;
	POOL_ACTION_OPT         cso_look;
	struct free_queue_list  qlist;
	bool                    sync;	/*true 同步，false异步*/
	void                    *data;
};

int xpool_create(const char *name, int max, bool sync,
	POOL_ACTION_OPT cso_open, POOL_ACTION_OPT cso_exit, POOL_ACTION_OPT cso_look, void *data);

/**
 * 通过名称获取池对象
 * @param name 池名称
 * @return 池对象，返回null，则表示不存在，但不会设置errno
 * 不能主动释放获取到的指针
 */
struct xpool    *xpool_gain(const char *name);

/**
 * 销毁池
 * @param name 池名称
 * @param data 销毁池中对象的回调入参
 */
void xpool_destroy(const char *name, void *data);

/**
 * 通过池对象获取成员
 * @param pool 池对象
 * @param cite 如果成功，成员被存入在此地址中
 * @param data 如果池中没有成员且池未满，则此参数传入创造函数或检查函数，然后调用
 * @return 返回0时，表示成功，<0时，表示失败，且设置errno，一般为ENOMEM(池已满)，或创造成员函数设置的错误
 * 如果池被设置为阻塞获取，则一直阻塞到获取一个成员，或创造成员失败退出
 */
int xpool_element_pull(struct xpool *pool, void **cite, void *data);

/**
 * 通过池对象归还成员
 * @param pool 池对象
 * @param cite 归还的成员
 */
int xpool_element_push(struct xpool *pool, void **cite, void *data);

/**
 * 通过池对象销毁成员，在成员状态错误且不能再用时调用此函数
 * @param pool 池对象
 * @param cite 归还的成员
 * @param data 此参数传入销毁函数，然后调用
 * @return 返回值为销毁函数的返回值
 */
int xpool_element_free(struct xpool *pool, void **cite, void *data);

/**
 * 通过池名称获取成员
 */
int xpool_gain_element(const char *name, void **cite, void *data);

