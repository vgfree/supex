#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/alive_api.h"
#include "minor/sniff_api.h"
#include "alive_cpp_api.h"

#include "sniff_evcoro_lua_api.h"



extern struct sniff_cfg_list g_sniff_cfg_list;


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

// start
	key = // 从客户端接收的数据提取key.
	fd = hkey_get_fd(key); // 通过接收到的key 寻找cid.
	if (fd != -1) // 如该cid 存在， 那么可能之前有数据存储.
	{
		struct residue_package package;
		char *value = hkey_get_value(key); //从对应的libkv 中取得之前存的package.
		int offset = hkey_get_offset(key); //数据的偏移.
		init_residue_package(&package, task->cid.fd, offset, value, strlen(value)); // 不支持二进制.
		push_residue_package(&package);// 把package 重新存到io thread 的buffer.	
		hkey_del_fd(key); // 把之前存储的所有对应内容清空.
		hkey_del_value(key);
		hkey_del_fd_key(fd);
		destroy_residue_package(&package);
	}
	else {
		//如果不存在，那么是第一次握手，不用处理.
	}
	hkey_insert_fd(key, cid.fd); // 存key, cid. 更新对应的key 与cid 的bind.
// end
	return 0;
}

int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
// start 
// 不用处理
// end
	return 0;
}

int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
       //start
	   struct residue_package package; // 用于提取来自io thread 中的buffer.
	   pop_residue_pakage(&package);  // 从io thread 提取.
	   char *key = hkey_find_key(task->cid.fd); // 从cid 中找对应的key.
	   hkey_insert_value(key, &package);  // 把key 对应的package 存储起来.
	   //end
	return 0;
}

