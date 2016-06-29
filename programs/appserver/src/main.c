/*
 * CopyRight    : DT+
 * Author       : louis.tin
 * Date         : 06-28-2016
 * Description  : Appserver
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <uuid/uuid.h>
#include <time.h>
#include <pthread.h>

#include "load_cfg.h"
#include "major/smart_api.h"
#include "appsrv.h"
#include "json.h"

struct smart_cfg_list g_smart_cfg_list = {};

// 客户端登录时发送的cid
char load_cid[256] = {};

void get_uid(struct app_msg recv_msg);
void set_uidmap(struct app_msg recv_msg);
void *t_msg_recv();
int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int main(int argc, char **argv)
{
	create_io();

	// 创建线程接收客户端数据
	pthread_t tid_recv;
	int ret = 0;
	ret = pthread_create(&tid_recv, NULL, (void *)t_msg_recv, NULL);
	if (ret != 0) {
		printf("Create send pthread error!\n");
		return -1;
	}

	load_supex_args(&g_smart_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	g_smart_cfg_list.func_info[SET_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[SET_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_call_set;

	smart_mount(&g_smart_cfg_list);
	smart_start();

	return 0;
}

void get_uid(struct app_msg recv_msg)
{
	printf("下发数据: 获取uid\n");
	char *downstream = "downstream";
	char *cid = "cid";
	char cid_str[20] = {};
	memcpy(cid_str, recv_msg.vector[2].iov_base, recv_msg.vector[2].iov_len);
	memcpy(load_cid, recv_msg.vector[2].iov_base, recv_msg.vector[2].iov_len);
	char *msg = "bind";
	printf("cid = %s, cid_str = %s, msg = %s\n", cid, cid_str, msg);

	struct app_msg send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = downstream;
	send_msg.vector[0].iov_len = strlen(downstream);
	send_msg.vector[1].iov_base = cid;
	send_msg.vector[1].iov_len = strlen(cid);
	send_msg.vector[2].iov_base = cid_str;
	send_msg.vector[2].iov_len = strlen(cid_str);
	send_msg.vector[3].iov_base = msg;
	send_msg.vector[3].iov_len = strlen(msg);
	send_app_msg(&send_msg);
}

void set_uidmap(struct app_msg recv_msg)
{
	printf("下发数据: 绑定uidmap\n");
	// 从接收到的第四帧数据json中解析出uid
	struct json_object *obj = NULL;
	struct json_object *new_obj = json_tokener_parse(recv_msg.vector[3].iov_base);
	char uid[64];

	if (json_object_object_get_ex(new_obj, "uid", &obj))
		strcpy(uid, json_object_get_string(obj));

	printf("uid = %s\n", uid);

	char            setting[] = "setting";
	char            uidmap[] = "uidmap";
	char            cid[32] = {};
	memcpy(cid, recv_msg.vector[1].iov_base, recv_msg.vector[1].iov_len);

	struct app_msg  send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = setting;
	send_msg.vector[0].iov_len = strlen(setting);
	send_msg.vector[1].iov_base = uidmap;
	send_msg.vector[1].iov_len = strlen(uidmap);
	send_msg.vector[2].iov_base = cid;
	send_msg.vector[2].iov_len = strlen(cid);
	send_msg.vector[3].iov_base = uid;
	send_msg.vector[3].iov_len = strlen(uid);
	send_app_msg(&send_msg);
}

void *t_msg_recv()
{
	while (1) {
		struct app_msg recv_msg = {};
		int more = 0;
		recv_app_msg(&recv_msg, &more, -1);

		printf("recv_msg size:%d\t [0]iov_len:%d\t [0]iov_base = %s,\n",
				recv_msg.vector_size, recv_msg.vector[0].iov_len, recv_msg.vector[0].iov_base);
		printf("recv_msg size:%d\t [1]iov_len:%d\t [1]iov_base = %s,\n",
				recv_msg.vector_size, recv_msg.vector[1].iov_len, recv_msg.vector[1].iov_base);
		printf("recv_msg size:%d\t [2]iov_len:%d\t [2]iov_base = %s,\n",
				recv_msg.vector_size, recv_msg.vector[2].iov_len, recv_msg.vector[2].iov_base);
		printf("recv_msg size:%d\t [3]iov_len:%d\t [3]iov_base = %s,\n",
				recv_msg.vector_size, recv_msg.vector[3].iov_len, recv_msg.vector[3].iov_base);
		assert(recv_msg.vector_size > 0);

		// 解析接收到的数据
		if (memcmp(recv_msg.vector[0].iov_base, "status", 6) == 0) {
			if (memcmp(recv_msg.vector[1].iov_base, "connected", 9) == 0) {
				// 下发数据获取uid
				get_uid(recv_msg);
			}
		} else if (memcmp(recv_msg.vector[0].iov_base, "upstream", 8) == 0) {
			if (memcmp(recv_msg.vector[2].iov_base, "bind", 4) == 0) {
				// 发送给setting server 设置uidmap
				set_uidmap(recv_msg);
			}
		} else {
			printf("recv_msg.vector[0].iov_base = %s\n", recv_msg.vector[0].iov_base);
		}
	}
}

int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	char msg[64];

	struct data_node *p_node = get_pool_data(task->sfd);
	char *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	//printf("p_buf = %s\n", p_buf);

	struct redis_status *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	//printf("len: %d \n", p_rst->field[1].len);
	//printf("command_type: %x \n", p_rst->command_type);

	memcpy(msg, p_buf + p_rst->field[1].offset, MIN(p_rst->field[1].len, 64 - 1));

	// 在登录后通过redis协议向客户端下发接收的redis数据
	struct app_msg  send_msg = {};

	char *downstream = "downstream";
	char *cid = "cid";
	printf("load_cid = %s, msg = %s\n", load_cid, msg);

	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = downstream;
	send_msg.vector[0].iov_len = strlen(downstream);
	send_msg.vector[1].iov_base = cid;
	send_msg.vector[1].iov_len = strlen(cid);
	send_msg.vector[2].iov_base = load_cid;
	send_msg.vector[2].iov_len = strlen(load_cid);
	send_msg.vector[3].iov_base = msg;
	send_msg.vector[3].iov_len = strlen(msg);

#if 0
	// 向组gid0下发数据
	char downstream[] = "downstream";
	char gid[] = "gid";
	char gid0[] = "gid0";

	printf("msg = %s\n", msg);

	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = downstream;
	send_msg.vector[0].iov_len = strlen(downstream);
	send_msg.vector[1].iov_base = gid;
	send_msg.vector[1].iov_len = strlen(gid);
	send_msg.vector[2].iov_base = gid0;
	send_msg.vector[2].iov_len = strlen(gid0);
	send_msg.vector[3].iov_base = msg;
	send_msg.vector[3].iov_len = strlen(msg);
#endif
	send_app_msg(&send_msg);

	// server端向请求端回复
	const char sndcnt[] = ":1\r\n";
	cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);
}

/*
 * 数据帧格式
 *
 * Login server 发出的命令
 * 0 status
 * 1 [connected]/[closed]
 * 2 CID
 *
 * 发送给Setting server 的命令
 * 0 setting
 * 1 [status]/[uidmap]/[gidmap]
 * 2 CID
 * 3 [closed]/[uid]/[gid]
 *
 * 普通消息下发
 * 0 downstream
 * 1 [cid]/[uid]/[gid]
 * 2 CID/UID/GID
 *
 * bind 消息下发
 * 0 downstream
 * 1 [cid]
 * 2 CID
 * 3 [bind]
 *
 * bind 消息上行
 * 0 upstream
 * 1 CID
 * 2 [bind]
 * 3 {UID}
 */




