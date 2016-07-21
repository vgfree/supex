#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/alive_api.h"
#include "minor/sniff_api.h"
#include "alive_cpp_api.h"
#include "keyval.h"

#include "sniff_evcoro_lua_api.h"

#include "mfptp_protocol/mfptp_package.h"
#include "comm_structure.h"
#include "comm_disposedata.h"


extern struct sniff_cfg_list g_sniff_cfg_list;

/* @pckbuff_size: 值-结果参数，传进来时代表的是保存打包成功数据的缓冲区大小 出去时代表的是缓冲去已有数据的大小 */
bool pair_packager(char **pckbuff, int *pckbuff_size, int pair_size)
{
	struct mfptp_packager   packager = {};	/* 打包器结构体 */
	mfptp_package_init(&packager, pckbuff, pckbuff_size);
	
	char    data[] = {0x12, 0x14, 0x15};		/* 待打包的数据 */
	int     dsize = sizeof(data);			/* 待打包数据的总大小 */
	int     packages = 1;				/* 待打包的总包数 */
	int	frames[1] = {1};			/* 待打包的总帧数 */
	int	frame_size[64] = { sizeof(data) };	/* 每帧的数据大小 */
	int	frame_offset[46] = { 0 };		/* 每帧的数据偏移 */
	
	/*检查空间是否足够*/
	int size = mfptp_check_memory(*pckbuff_size, frames[0], dsize);
	assert(size == 0);
	
	*pckbuff_size = 0;	/* 从此刻起代表的就是缓冲区已有数据的大小 */
	mfptp_fill_package(&packager, frame_offset, frame_size, frames, packages);
	size = mfptp_package(&packager, data, NO_ENCRYPTION|NO_COMPRESSION, PAIR_METHOD);
	if ((size > 0) && (packager.ms.error == MFPTP_OK)) {
		printf("packager successed\n");
		return true;
	} else {
		printf("packager failed\n");
		return false;
	}
}

/*
 * key:DATA  ------>   val
 * sfd:MARK  ------>   key
 */
int alive_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	printf("size %d data %s\n", task->size, task->data);

	ALIVE_WORKER_PTHREAD    *p_alive_worker = (ALIVE_WORKER_PTHREAD *)user;

	if (task->size == 0) {
		// TODO
		// x_printf
		return -1;
	}

	if (task->size > MAX_SNIFF_DATA_SIZE) {
		// TODO
		// x_printf
		return -1;
	}
#if 0
	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;
	sniff_task.last = false;
	sniff_task.stamp = time(NULL);
	sniff_task.size = task->size;
	memcpy(sniff_task.data, (const char *)task->data, task->size);

	alive_send_data(true, task->cid, task->sfd, "good", 5);
#endif

	struct mfptp_parse_info *mfptp_info = task->parse.mfptp_info;
	if (mfptp_info->header.socket_type == PAIR_METHOD) {
		// 从客户端接收的数据提取key.
		char *key = task->data + mfptp_info->bodyer.package[0].frame[0].frame_offset;
		int len = mfptp_info->bodyer.package[0].frame[0].frame_size;
		// 通过接收到的key 寻找val.
		int size = 0;
		char *val = key_get_val(key, &size);
		if (val) {
			struct data_node *p_node = get_pool_data(task->sfd);
			cache_append(&p_node->mdl_up.cache, val, size);
			key_del_val(key);

			// 组装pair返回数据
			char	buff[1024] = {};		/* 保存打包成功的数据缓冲区 */
			char*	pckbuff = buff;
			int	pckbuff_size = 1024;		/* 保存打包成功数据缓冲区大小 */

			bool ok = pair_packager(&pckbuff, &pckbuff_size, size);
			assert(ok);
			alive_send_data(true, task->cid, task->sfd, pckbuff, pckbuff_size);
			

			free(val);
		} else {
			// 组装pair返回数据
			char	buff[1024] = {};		/* 保存打包成功的数据缓冲区 */
			char*	pckbuff = buff;
			int	pckbuff_size = 1024;		/* 保存打包成功数据缓冲区大小 */

			bool ok = pair_packager(&pckbuff, &pckbuff_size, 0);
			assert(ok);
			alive_send_data(true, task->cid, task->sfd, pckbuff, pckbuff_size);
		}
		sfd_del_key(task->sfd);
		sfd_set_key(task->sfd, key);
	}
	return 0;
}

int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return 0;
}

int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node *p_node = get_pool_data(task->sfd);
	struct cache *cache = &p_node->mdl_up.cache;
	char *val = cache_data_length(cache);
	int size = cache_data_address(cache);

	char *key = sfd_get_key(task->sfd);
	if (size > 0) {
		key_set_val(key, val, size);
		//TODO expire
		free(key);
	}
	return 0;
}

