#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include "node_manager.h"
// #include "utils.h"
// #include "atomic.h"
#include "malloc_hook.h"

#define GMAXLAYERS (INRANGE(MANAGER_MAX_LAYER, 1, sizeof(int) * 8))

#ifndef real_calloc
  #define real_calloc(n, size) (calloc((n), (size)))
#endif

/*
 * 内存管理器节点定义
 */
typedef struct _NodeManagerT *NodeManagerT;

/*
 * 内存管理器链表定义
 */
typedef struct _ManagerListT ManagerListT;

/*
 * 内存管理器定义
 */
typedef struct _ManagerT ManagerT;

/*
 * 内存管理器节点定义
 */
struct _NodeManagerT
{
	void            *ptr;				/**< 动态分配的内存指针*/
	long            bytes;				/**< 动态分配的内存长度*/
#ifdef _GNU_SOURCE
	char            func[32];			/**< 动态分配的函数*/
	char            file[32];			/**< 动态分配的文件*/
	void            *calladdr;			/**< 调用地址*/
	int             line;				/**< 动态分配的行数*/
#endif
	long            owner;				/**< 线程拥有者*/
	NodeManagerT    next[1];			/**< 跳表的下一个路径*/
};

/*
 * 内存管理器链表定义
 */
struct _ManagerListT
{
	long            layer;				/**< 跳表层数*/
	long            count;				/**< 节点数*/
	AO_SpinLockT    lock;				/**< 同步操作锁*/
	NodeManagerT    head;				/**< 跳表头指针*/
};

/*
 * 内存管理器定义
 */
struct _ManagerT
{
	int             magic;					/**< 初始化魔数*/
	AO_T            total;					/**< 节点总数*/
	ManagerListT    bucket[MANAGER_MAX_BUCKETS];		/**< 哈希桶*/
};

/* ------------------------                   */

int gmhookTraceLogFD = STDOUT_FILENO;

/*
 * 实现一个跳表作为内存检查，KEY为内存指针，VALUE为内存信息
 */
static ManagerT gManager = {};

/*
 * 最大层数
 */
static const long gMaxLayers = GMAXLAYERS;

/*
 * 节点查找路径
 */
static __thread NodeManagerT gUpdatePath[GMAXLAYERS] = {};

/*
 * 哈希宏
 */
#define         _HASH_POINTER(ptr) \
	((int)(((uintptr_t)(ptr) >> 3) % DIM(gManager.bucket)))

/*
 * 定位操作链表
 */
#define         _ManagerPositionList(ptr) \
	(&gManager.bucket[_HASH_POINTER((ptr))])

/*
 * 分配一个新的节点
 */
#define         _NewManagerNode(pptr, layer) \
	(*(pptr) = (__typeof__(*(pptr)))     \
	real_calloc(1, sizeof(**(pptr)) + ((layer) - 1) * sizeof(*(pptr))))

/*
 * 私有函数定义
 */
static
long _RandomLayer();

/*
 * 初始化函数
 */
#if defined(__GNUC__)

static
void _ManagerInitCB() __attribute__((constructor(101)));

  #define       _ManagerInit() ((void)0)

#else

/*
 * 进程初始化标志
 */
static
pthread_once_t gManagerOnce = PTHREAD_ONCE_INIT;

static
void _ManagerInitCB();

  #define       _ManagerInit() \
	pthread_once(&gManagerOnce, _ManagerInitCB)
#endif

static
NodeManagerT _ManagerFindNode(ManagerListT *list, void *ptr);

static inline
void _FillBackTraceNode(void *btinfo, NodeManagerT node);

/*
 * 随机得出一个层数
 */
static
long _RandomLayer()
{
	long    layer = 0;
	double  rv = MANAGER_RAND_SEED;

	assert(rv < 1 && rv > 0);

	/*
	 * 要保证layer在循环结束后不能大于 MANAGER_MAX_LAYER
	 * 要增加跳表每个节点的层数，请增加几率
	 */
	for (layer = 1; layer < gMaxLayers && RandChance(rv); ) {
		layer++;
	}

	return layer;
}

/*
 * 内存管理器初始化函数
 */
static
void _ManagerInitCB()
{
	int             i = 0;
	int             fd = -1;
	ManagerListT    *list = NULL;

	bzero((void *)&gManager.bucket[0], DIM(gManager.bucket));

	gManager.total = 0;

	Rand();

#ifndef MHOOKTRACELOG
  #define MHOOKTRACELOG "./mhooktrace.log"
#endif
	fd = open(MHOOKTRACELOG, O_CREAT | O_WRONLY | O_APPEND, 0644);

	if (fd < 0) {
		x_printf(E, "Can't open %s : %s\n", MHOOKTRACELOG, strerror(errno));
	} else {
		ATOMIC_SET(&gmhookTraceLogFD, fd);
	}

	/*
	 * 分配每个HASH桶的数据
	 */
	for (i = 0; i < DIM(gManager.bucket); i++) {
		AO_SpinLockInit(&gManager.bucket[i].lock, false);
		list = &gManager.bucket[i];

		_NewManagerNode(&list->head, gMaxLayers);

		assert(&list->head);
	}

	gManager.magic = OBJMAGIC;
}

/*
 * 查找一个节点，存入线程私有路径数组中
 */
static
NodeManagerT _ManagerFindNode(ManagerListT *list, void *ptr)
{
	long            layer = 0;
	NodeManagerT    head = NULL;
	NodeManagerT    search = NULL;

	assert(list);
	assert(list->layer >= 0);

	/*
	 * 从最高层开始搜索
	 */
	for (layer = list->layer, head = list->head, search = NULL;
		layer-- > 0; ) {
		/*
		 * 搜索当前层的链表
		 */
		while ((search = head->next[layer]) && search->ptr < ptr) {
			head = search;
		}

		/*
		 * 保存当前路径
		 */
		gUpdatePath[layer] = head;
	}

	if (search && (search->ptr == ptr)) {
		return search;
	}

	return NULL;
}

static inline
void _FillBackTraceNode(void *btinfo, NodeManagerT node)
{
	assert(node);

#ifdef _GNU_SOURCE
  #include <dlfcn.h>

	if (btinfo) {
		/* code */
		const char      *ptr1 = NULL;
		const char      *ptr2 = NULL;
		Dl_info         *dli = (Dl_info *)btinfo;

		node->calladdr = dli->dli_saddr;
		ptr1 = SWITCH_NULL_STR(dli->dli_fname);
		ptr2 = strrchr(ptr1, '/');
		snprintf(node->file, sizeof(node->file), "%s", ptr2 ? ptr2 + 1 : ptr1);

		ptr1 = SWITCH_NULL_STR(dli->dli_sname);
		ptr2 = strrchr(ptr1, '/');
		snprintf(node->func, sizeof(node->func), "%s", ptr2 ? ptr2 + 1 : ptr1);
	}
#endif
}

bool PushManager(void *ptr, long nbytes, void *btinfo)
{
	NodeManagerT    node = NULL;
	ManagerListT    *list = NULL;
	long            layer = 1;
	bool            flag = false;

	_ManagerInit();

	list = _ManagerPositionList(ptr);

	AO_SpinLock(&list->lock);

	/*
	 * 搜索节点
	 */
	node = _ManagerFindNode(list, ptr);

	if (node) {
		AO_SpinUnlock(&list->lock);
		return false;
	}

	/*
	 * 从1开始、按1/2几率获取一个层数
	 */
	layer = _RandomLayer();

	/*
	 *  每次只多增加一层
	 */
	if (layer > list->layer) {
		gUpdatePath[list->layer] = list->head;
		layer = ++list->layer;
	}

	_NewManagerNode(&node, layer);

	if (node) {
		node->ptr = ptr;
		node->bytes = nbytes;
		node->owner = GetThreadID();

		/*
		 * 填充栈的调用信息到节点
		 */

		_FillBackTraceNode(btinfo, node);

		/*
		 * 每层的插入点就是路径节点
		 */
		while (layer-- > 0) {
			node->next[layer] = gUpdatePath[layer]->next[layer];
			gUpdatePath[layer]->next[layer] = node;
		}

		list->count++;

		ATOMIC_INC(&gManager.total);

		flag = true;
	}

	AO_SpinUnlock(&list->lock);

	return flag;
}

bool PopManager(void *ptr, long *nbytes, void *btinfo)
{
	NodeManagerT    node = NULL;
	ManagerListT    *list = NULL;
	long            layer = 0;

	_ManagerInit();

	list = _ManagerPositionList(ptr);

	AO_SpinLock(&list->lock);

	node = _ManagerFindNode(list, ptr);

	if (!node) {
		AO_SpinUnlock(&list->lock);
		return false;
	}

	/*
	 * 将路径上的每一层的每个节点的信息更新
	 */
	for (layer = 0; layer < list->layer &&
		gUpdatePath[layer]->next[layer] == node; layer++) {
		gUpdatePath[layer]->next[layer] = node->next[layer];
	}

	SET_POINTER(nbytes, node->bytes);

	real_free(node);

	/*
	 * 如果最高层的节点被删除，需要降低跳表的当前层数到正确值
	 */
	while (list->layer && list->head->next[list->layer - 1] == NULL) {
		list->layer--;
	}

	list->count--;

	ATOMIC_DEC(&gManager.total);

	AO_SpinUnlock(&list->lock);

	return true;
}

bool PullManager(void *ptr, long *nbytes)
{
	NodeManagerT    node = NULL;
	ManagerListT    *list = NULL;

	_ManagerInit();

	assert(gManager.magic == OBJMAGIC);

	list = _ManagerPositionList(ptr);

	AO_SpinLock(&list->lock);

	node = _ManagerFindNode(list, ptr);

	AO_SpinUnlock(&list->lock);

	if (!node) {
		return false;
	}

	SET_POINTER(nbytes, node->bytes);

	return true;
}

long ManagerTotal()
{
	_ManagerInit();

	assert(gManager.magic == OBJMAGIC);

	return gManager.total;
}

void ManagerPrint(const char *file, int fd)
{
	long    i = 0;
	int     desc = -1;
	int     stat = -1;
	int     ret = -1;
	bool    flag = false;

	_ManagerInit();

	assert(gManager.magic == OBJMAGIC);

	if (file) {
		desc = open(file, O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0644);

		if (desc < 0) {
			x_printf(S, "Can't open %s : %s\n", file, strerror(errno));
		}

		flag = true;
	} else if ((stat = fcntl(fd, F_GETFL)) > 0) {
		if (((stat & O_ACCMODE) == O_WRONLY) ||
			((stat & O_ACCMODE) == O_RDWR)) {
			desc = fd;
		}
	}

	desc = desc < 0 ? STDOUT_FILENO : desc;

	/*打印头*/
#ifdef _GNU_SOURCE
	ret = dprintf(desc, "%20s|%32s|%32s|%20s|%20s|%20s\n",
			"thread id", "process", "function",
			"call address", "memory address",
			"memory size");
#else
	ret = dprintf(desc, "%20s|%20s|%20s\n",
			"thread id", "memory address",
			"memory size");
#endif

	if (ret < 0) {
		return;
	}

	/*
	 * 遍历桶
	 */
	for (i = 0; i < DIM(gManager.bucket); i++) {
		/*
		 * 遍历桶中节点
		 */
		ret = 0;
		ManagerListT    *list = NULL;
		NodeManagerT    next = NULL;

		list = (ManagerListT *)&gManager.bucket[i];

		AO_SpinLock(&list->lock);

		for (next = list->head->next[0]; next; next = next->next[0]) {
#ifdef  _GNU_SOURCE
			ret = dprintf(desc, "%8ld|%32s|%32s|%20p|%20p|%20ld\n",
					next->owner, next->file,
					next->func, next->calladdr,
					next->ptr, next->bytes);
#else
			ret = dprintf(desc, "%8ld|%20p|%20ld\n",
					next->owner, next->ptr, next->bytes);
#endif

			if (ret < 0) {
				break;
			}
		}

		AO_SpinUnlock(&list->lock);

		if (ret < 0) {
			break;
		}
	}

	if (flag) {
		close(desc);
	}
}

