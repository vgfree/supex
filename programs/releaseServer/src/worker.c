#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <ev.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "cq_list.h"
#include "worker.h"
#include "data_json.h"
#include "worker_mem.h"
#include "grp_pthread.h"
#include "jtt_business.h"
#include "log.h"

int worker_data_handle(char *buf, int buflen)
{
	char    imei[16] = {};
	char    thread_idx = -1;
	char    who[16] = {};
	NODE    *node = NULL;
	cJSON   *json = NULL;

	log_info(LOG_I, "buf[%s]\n", buf);

	/*从json字符串中获得imei*/
	if (NULL == (json = json_imei_get(buf, imei, sizeof(imei)))) {
		log_info(LOG_E, "http数据中找不到imei\n");
		return -1;
	}

	/*过滤并根据imei获得通信协议类型*/
	if (-1 == mem_thread_get(imei, &thread_idx, who)) {
		log_info(LOG_I, "DEBUGE过滤掉imei[%s]\n", imei);
		cJSON_Delete(json);
		return 0;
	}

	log_info(LOG_I, "imei[%s],who[%s],接出线程编号thread_idx[%d]\n", imei, who, thread_idx);

	/*根据协议类型选择队列分配任务*/
	node = (NODE *)calloc(1, sizeof(NODE));

	if (NULL == node) {
		log_info(LOG_E, "内存分配错\n");
		cJSON_Delete(json);
		return -1;
	}

	strncpy(node->userid, who, sizeof(node->userid) - 1);
	node->data = json;

	if (-1 == grp_thread_push(thread_idx, node)) {
		log_info(LOG_E, "接出线程thread_idx[%d]的数据分配给接出线程群错误\n", thread_idx);
		cJSON_Delete(json);
		json = NULL;
		free(node);
		return -1;
	}

	return 0;
}

