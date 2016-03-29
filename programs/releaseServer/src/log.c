/*
 *   模块说明：
 *        线程日志：在指定文件名后面加一个线程号，构成线程的私有日志文件
 *        日志特点：
 *                1、每个线程一个日志文件
 *                2、文件超过一定大小，备份
 *                3、备份文件超过一定数量，删除最久远的备份文件
 *                4、日志文件删除后，可重建
 *
 *   作者：陶涛
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>

#define LOG_FILE_MAX_SIZE       (1024 * 1024 * 300)	/*单个日子文件最大大小100M*/
#define DEL_LOG_CNT_MAX         20			/*默认的最大日志文件数*/
#define LOG_BACKUP_MAX          4			/*每个日志的最大备份数量*/

typedef struct _LOG_NODE
{
	pid_t   id;				/*线程id，日志文件属于哪个线程*/
	char    file[512];			/*日志文件名*/
	int     fd;				/*文件描述符*/

	int     rmidx;				/*最久远备份文件,即待删除的文件下标*/
	char    *backfile[LOG_BACKUP_MAX];	/*备份文件名*/
} LOG_NODE;

typedef struct _DO_ONCE
{
	int     lock;
	int     step;
} DO_ONCE;
static DO_ONCE do_once = { .0, .0 };

int g_log_level = 0;						/*外部配置的日志级别*/

unsigned        g_log_max = DEL_LOG_CNT_MAX;			/*外部赋值的日志文件数量*/
static LOG_NODE g_default_log_tab[DEL_LOG_CNT_MAX] = {};	/*默认日志文件表*/

static LOG_NODE *g_log_tab = g_default_log_tab;			/*日志文件信息表*/
static unsigned g_log_cur = 0;					/*表中当前可用节点的下标,g_log_cur<g_log_max*/

int log_init(char *file)
{
	unsigned        cur;
	LOG_NODE        node = {};

	/*内存初始化只做一次*/
	while (!__sync_bool_compare_and_swap(&do_once.lock, 0, 1)) {}

	if (!__sync_fetch_and_add(&do_once.step, 1)) {
		g_log_tab = (LOG_NODE *)calloc(g_log_max, sizeof(LOG_NODE));

		if (!g_log_tab) {
			g_log_tab = g_default_log_tab;
			g_log_max = DEL_LOG_CNT_MAX;
		}
	}

	assert(__sync_bool_compare_and_swap(&do_once.lock, 1, 0));

	cur = __sync_fetch_and_add(&g_log_cur, 1);

	if (cur >= g_log_max) {
		__sync_fetch_and_sub(&g_log_cur, 1);
		return -1;
	}

	node.id = syscall(SYS_gettid);
	snprintf(node.file, sizeof(node.file), "%s.%d", file, node.id);

	node.fd = open(node.file, O_WRONLY | O_APPEND | O_CREAT, 0644);

	if (-1 == node.fd) {
		return -1;
	}

	g_log_tab[cur] = node;

	return 0;
}

int idx_get(void)
{
	int     i;
	pid_t   id = syscall(SYS_gettid);
	int     cur;

	cur = g_log_cur;

	for (i = 0; i <= cur; i++) {
		if (id == g_log_tab[i].id) {
			return i;
		}
	}

	return -1;
}

static char *sys_time(char *buf, int len)
{
	char    fmt[20] = "%Y%m%d%H%M%S";
	time_t  tm;

	if (!buf || (len < 20)) {
		return NULL;
	}

	time(&tm);

	strftime(buf, len, fmt, localtime(&tm));

	return buf;
}

static char *sys_utime(char *buf, int len)
{
	char            fmt[32] = "%Y/%m/%d-%H:%M:%S:";
	struct timeval  tv;
	int             length;

	if (!buf || (len < 32)) {
		return NULL;
	}

	gettimeofday(&tv, NULL);

	strftime(buf, len, fmt, localtime(&tv.tv_sec));
	length = strlen(buf);
	snprintf(buf + length, len - length, "%lu", tv.tv_usec);

	return buf;
}

static void set_backup_file(LOG_NODE *node, char *backfile)
{
	int i;

	/*满*/
	if (node->backfile[LOG_BACKUP_MAX - 1]) {
		if (0 == strcmp(node->backfile[node->rmidx], backfile)) {
			strcat(backfile, "s");
		}

		remove(node->backfile[node->rmidx]);
		strcpy(node->backfile[node->rmidx], backfile);
		node->rmidx = (node->rmidx + 1) % LOG_BACKUP_MAX;
		return;
	}

	for (i = 0; i < LOG_BACKUP_MAX; i++) {
		if (!node->backfile[i]) {
			if (i > 0) {
				if (0 == strcmp(node->backfile[i - 1], backfile)) {
					strcat(backfile, "s");
				}
			}

			node->backfile[i] = (char *)calloc(1, 512);

			if (!node->backfile[i]) {
				return;
			}

			strcpy(node->backfile[i], backfile);
			return;
		}
	}
}

/*
 *   功能：按级别打印日志
 */
void log_info(int level, const char *file, const char *func, int line, char *fmt, ...)
{
	int             i;
	int             fd;
	char            lv[16] = {};
	char            buf[32] = {};
	va_list         va;
	struct stat     files;
	char            file_name[512] = {};

	if (level < g_log_level) {
		return;
	}

	switch (level)
	{
		case 0:
			strcpy(lv, LOG_DEBUG);
			break;

		case 1:
			strcpy(lv, LOG_INFO);
			break;

		case 2:
			strcpy(lv, LOG_WARN);
			break;

		case 3:
			strcpy(lv, LOG_FAIL);
			break;

		case 4:
			strcpy(lv, LOG_ERR);
			break;

		case 5:
			strcpy(lv, LOG_SYSI);
			break;

		case 6:
			strcpy(lv, LOG_SYSE);
			break;

		default:
			return;
	}

	i = idx_get();
	fd = (-1 == i) ? 1 : g_log_tab[i].fd;		/*不能成功获得fd时，直接打印到标准输出*/

	if (fd > 2) {
		if (0 != access(g_log_tab[i].file, F_OK)) {
			close(g_log_tab[i].fd);
			g_log_tab[i].fd = open(g_log_tab[i].file, O_WRONLY | O_APPEND | O_CREAT, 0644);

			if (-1 == g_log_tab[i].fd) {
				g_log_tab[i].fd = 1;
			}

			fd = g_log_tab[i].fd;
		}

		fstat(fd, &files);

		if (files.st_size > LOG_FILE_MAX_SIZE) {
			char tv[32] = {};

			fsync(g_log_tab[i].fd);
			close(g_log_tab[i].fd);
			sys_time(tv, sizeof(tv));
			snprintf(file_name, sizeof(file_name), "%s.%s", g_log_tab[i].file, tv);
			set_backup_file(&g_log_tab[i], file_name);
			rename(g_log_tab[i].file, file_name);

			g_log_tab[i].fd = open(g_log_tab[i].file, O_WRONLY | O_APPEND | O_CREAT, 0644);

			if (-1 == g_log_tab[i].fd) {
				g_log_tab[i].fd = 1;
			}

			fd = g_log_tab[i].fd;
		}
	}

	sys_utime(buf, sizeof(buf));
	dprintf(fd, "[%s]%s|%s(%s)|%d|--->", lv, buf, file, func, line);

	va_start(va, fmt);
	vdprintf(fd, fmt, va);
	va_end(va);
}

