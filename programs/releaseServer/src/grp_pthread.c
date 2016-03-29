/*
 *   ========================================================================
 *   模块说明：
 *        接出线程群模块
 *
 *   ========================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "out_pthread.h"
#include "lock.h"
#include "cq_list.h"
#include "log.h"

#include "grp_pthread.h"

/*要求外部实现的客户化接口*/
extern int jtt809_out_once_init(void);

extern int jtt809_do_heart(time_t space, int thread_idx);

extern int jtt809_data_handle(const void *data);

#define JTT809_HEART_SPACE 60		/*心跳间隔时间s*/

typedef struct _MNG_THREAD
{
	char            idx;	/*线程编号*/
	OTHER_PTHREAD   *thrd;
} MNG_THREAD;

OTHER_PTHREAD   *g_thread_tab;
int             g_thread_cnt;
MNG_THREAD      *g_mng_thread_tab;
int             g_mng_thread_cnt;

/*
 *功能：为集合中的每个OTHER_PTHREAD线程进行初始化
 */
int
grp_thread_init(void)
{
	int i;

	// g_thread_cnt     = 3;//现由配置文件加载
	g_mng_thread_cnt = g_thread_cnt;

	g_thread_tab = (OTHER_PTHREAD *)calloc(g_thread_cnt, sizeof(OTHER_PTHREAD));

	if (NULL == g_thread_tab) {
		log_info(LOG_E, "内存分配失败\n");
		return -1;
	}

	g_mng_thread_tab = (MNG_THREAD *)calloc(g_mng_thread_cnt, sizeof(MNG_THREAD));

	if (NULL == g_mng_thread_tab) {
		log_info(LOG_E, "内存分配失败\n");
		return -1;
	}

	g_thread_tab[0].init = jtt809_out_once_init;

	/*809协议线程,配置*/
	for (i = 0; i < g_thread_cnt; i++) {
		g_thread_tab[i].do_heart = jtt809_do_heart;
		g_thread_tab[i].heart_space = JTT809_HEART_SPACE;
		g_thread_tab[i].thread_idx = i;
		g_thread_tab[i].data_handle = jtt809_data_handle;
		g_mng_thread_tab[i].idx = i;			/*几号线程,线程编号*/
		g_mng_thread_tab[i].thrd = &g_thread_tab[i];
		snprintf(g_thread_tab[i].logfile, sizeof(g_thread_tab[i].logfile), "%s/log/%s", getenv("WORK_PATH"), "jtt_business.log");

		if (-1 == cqu_init(&g_thread_tab[i].cq)) {
			log_info(LOG_E, "为809协议线程初始化队列失败\n");
			return -1;
		}

		if (pipe2(g_thread_tab[i].pfd, (O_NONBLOCK | O_CLOEXEC)) < 0) {
			log_info(LOG_SE, "启动out_pthread时，为其创建通信管道错误pipe2,strerror:%s\n", strerror(errno));
			return -1;
		}
	}

	/*初始化每个线程的业务信息*/
	if (-1 == g_thread_tab[0].init()) {
		log_info(LOG_E, "号线程初始化函数init执行失败\n");
		return -1;
	}

	return 0;
}

/*
 *功能：轮启集合中的每个线程
 * */
int
grp_thread_boot(void)
{
	int             i;
	WAIT            wait;
	SYNC_WAIT       sync_wait[g_thread_cnt];
	pthread_t       pthread_id;

	pthread_mutex_init(&wait.mutex, NULL);
	pthread_cond_init(&wait.cond, NULL);
	wait.count = 0;

	for (i = 0; i < g_thread_cnt; i++) {
		sync_wait[i].wait = &wait;
		sync_wait[i].data = &g_thread_tab[i];
		pthread_create(&pthread_id, NULL, out_pthread_start, &sync_wait[i]);
	}

	pthread_mutex_lock(&wait.mutex);

	while (wait.count < g_thread_cnt) {
		pthread_cond_wait(&wait.cond, &wait.mutex);
	}

	pthread_mutex_unlock(&wait.mutex);

	return 0;
}

/*
 *说明：在woker和线程集合之间提供联系方式
 */
int grp_thread_push(char idx, void *data)
{
	int     i;
	char    yes = 1;

	for (i = 0; i < g_mng_thread_cnt; i++) {
		if (idx == g_mng_thread_tab[i].idx) {
			if (-1 == cqu_push(g_mng_thread_tab[i].thrd->cq, data)) {
				log_info(LOG_F, "[%d]号接出线程的队列满了\n", i);
				return -1;
			}

			if (write(g_mng_thread_tab[i].thrd->pfd[1], &yes, sizeof(yes)) < 0) {
				log_info(LOG_SE, "写管道通知out_pthread[i]失败,----管道堆积\n", i);
				return -1;
			}

			return 0;
		}
	}

	return 0;
}

