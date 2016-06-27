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

char json_str[256];

char *get_json(char *cid, int msgTime, char *msgID)
{
	json_object *my_object;

	my_object = json_object_new_object();
	json_object_object_add(my_object, "opt", json_object_new_int("bind"));
	json_object_object_add(my_object, "CID", json_object_new_string(cid));
	json_object_object_add(my_object, "msgTime", json_object_new_boolean(msgTime));
	json_object_object_add(my_object, "msgID", json_object_new_boolean(msgID));

	printf("my_object=\n");
	json_object_object_foreach(my_object, key, val)
	{
		printf("\t%s: %s\n", key, json_object_to_json_string(val));
	}
	printf("my_object.to_string()=%s\n", json_object_to_json_string(my_object));

	//	json_object_put(my_object);
	strcpy(json_str, json_object_to_json_string(my_object));

	printf("json_str = %s", json_str);

	return json_str;
}

char *get_uuid()
{
	uuid_t  uuid;
	char    str[36];

	uuid_generate(uuid);
	uuid_unparse(uuid, str);

	return str;
}

int get_time()
{
	time_t t;

	t = time(0);

	return t;
}

int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	char msg[16];

	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);

	printf("p_buf = %s\n", p_buf);

	struct redis_status *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	printf("len: %d \n", p_rst->field[1].len);
	printf("command_type: %x \n", p_rst->command_type);

	memcpy(msg, p_buf + p_rst->field[1].offset, MIN(p_rst->field[1].len, 64 - 1));

	// 发送

	struct app_msg send_msg = {};

	char    downstream[] = "downstream";
	char    gid[] = "gid";
	char    gid0[] = "gid0";

	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = downstream;
	send_msg.vector[0].iov_len = strlen(downstream);
	send_msg.vector[1].iov_base = gid;
	send_msg.vector[1].iov_len = strlen(gid);
	send_msg.vector[2].iov_base = gid0;
	send_msg.vector[2].iov_len = strlen(gid0);
	send_msg.vector[3].iov_base = msg;
	send_msg.vector[3].iov_len = strlen(msg);

	send_app_msg(&send_msg);

	// server端给请求端回复内容
	const char sndcnt[] = ":1\r\n";
	cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);
}

void *t_msg_recv()
{
	// 循环接收数据
	while (1) {
		/*
		 *   // 接收数据
		 *   printf("接受数据");
		 *   struct app_msg recv_msg = {};
		 *   int more = 0;
		 *   recv_app_msg(&recv_msg, &more, -1);
		 *   printf("recv_msg size:%d, [0]iov_len:%d\n",
		 *   recv_msg.vector_size, recv_msg.vector[0].iov_len);
		 *
		 *   assert(recv_msg.vector_size > 0);
		 *
		 *   if (memcmp(recv_msg.vector[0].iov_base, "status", 6) == 0)
		 *   {
		 *   if (memcmp(recv_msg.vector[1].iov_base, "connected", 9) == 0)
		 *   {
		 *   // 发送请求UID
		 *   char downstream[] = "downstream";
		 *   char cid[] = "cid";
		 *   char cid_str[20] = {};
		 *   memcpy(cid_str, recv_msg.vector[2].iov_base, recv_msg.vector[2].iov_len);
		 *   char *msg;
		 *
		 *   char *uuid = get_uuid();
		 *   time_t time = get_time();
		 *
		 *   msg = get_json(cid, time, uuid);
		 *
		 *   struct app_msg  send_msg = {};
		 *   send_msg.vector_size = 4;
		 *   send_msg.vector[0].iov_base = downstream;
		 *   send_msg.vector[0].iov_len = strlen(downstream);
		 *   send_msg.vector[1].iov_base = cid;
		 *   send_msg.vector[1].iov_len = strlen(cid);
		 *   send_msg.vector[2].iov_base = cid_str;
		 *   send_msg.vector[2].iov_len = strlen(cid_str);
		 *   send_msg.vector[3].iov_base = msg;
		 *   send_msg.vector[3].iov_len = strlen(msg);
		 *   send_app_msg(&send_msg);
		 *   }
		 *   else if (memcmp(recv_msg.vector[1].iov_base, "closed", 6) == 0)
		 *   {
		 *
		 *   }
		 *   }
		 *   else
		 *   {
		 *   /\*
		 *   app-server先下发请求UID, 在接收到UID后, 再下发绑定cid-uid
		 * *\/
		 *   char setting[] = "setting";
		 *   char uidmap = "uidmap";
		 *   // 从返回值中获取UID, GID
		 *   struct json_object *obj = NULL;
		 *   char *json_string = json_object_to_json_string(recv_msg.vector[0].iov_base);
		 *   struct json_object *cfg = json_object_from_file(json_string);
		 *   char *cid = NULL;
		 *   char *uid = NULL;
		 *   if (json_object_object_get_ex(cfg, "CID", &obj))
		 *   {
		 *   strcpy(cid, json_object_get_string(obj));
		 *   }
		 *   else
		 *   {
		 *   printf("cid not found");
		 *   }
		 *
		 *   if (json_object_object_get_ex(cfg, "UID", &obj))
		 *   {
		 *   strcpy(uid, json_object_get_string(obj));
		 *   }
		 *   else
		 *   {
		 *        printf("uid not found");
		 *   }
		 *
		 *   struct app_msg  send_msg = {};
		 *   send_msg.vector_size = 4;
		 *   send_msg.vector[0].iov_base = setting;
		 *   send_msg.vector[0].iov_len = strlen(setting);
		 *   send_msg.vector[1].iov_base = uidmap;
		 *   send_msg.vector[1].iov_len = strlen(uidmap);
		 *   send_msg.vector[2].iov_base = cid;
		 *   send_msg.vector[2].iov_len = strlen(cid);
		 *   send_msg.vector[3].iov_base = uid;
		 *   send_msg.vector[3].iov_len = strlen(uid);
		 *   send_app_msg(&send_msg);
		 *   }
		 */
		struct app_msg  msg = {};
		int             more = 0;
		printf("start to recv msg.\n");
		recv_app_msg(&msg, &more, -1);
		printf("msg size:%d, [0]iov_len:%d\n",
			msg.vector_size, msg.vector[0].iov_len);
		assert(msg.vector_size > 0);

		printf("msg[0]:iov_len:%d\n", (int)msg.vector[0].iov_len);
		char buf[20] = {};
		memcpy(buf, msg.vector[0].iov_base, msg.vector[0].iov_len);
		printf("msg[0]:%s\n", buf);
		printf("msg[1]:iov_len:%d\n", (int)msg.vector[1].iov_len);

		if (memcmp(msg.vector[0].iov_base, "status", 6) == 0) {
			printf("status.");

			if (memcmp(msg.vector[1].iov_base, "connected", 9) == 0) {
				char            setting[] = "setting";
				char            gidmap[] = "gidmap";
				char            cid[20] = {}; memcpy(cid, msg.vector[2].iov_base, msg.vector[2].iov_len);
				char            gid[] = "gid0";
				struct app_msg  send_msg = {};
				send_msg.vector_size = 4;
				send_msg.vector[0].iov_base = setting;
				send_msg.vector[0].iov_len = strlen(setting);
				send_msg.vector[1].iov_base = gidmap;
				send_msg.vector[1].iov_len = strlen(gidmap);
				send_msg.vector[2].iov_base = cid;
				send_msg.vector[2].iov_len = strlen(cid);
				send_msg.vector[3].iov_base = gid;
				send_msg.vector[3].iov_len = strlen(gid);
				send_app_msg(&send_msg);
			}
		} else {
			struct app_msg send_msg = {};
			send_msg.vector_size = 3 + msg.vector_size;
			char    downstream[] = "downstream";
			char    gid[] = "gid";
			char    gid0[] = "gid0";
			send_msg.vector[0].iov_base = downstream;
			send_msg.vector[0].iov_len = strlen(downstream);
			send_msg.vector[1].iov_base = gid;
			send_msg.vector[1].iov_len = strlen(gid);
			send_msg.vector[2].iov_base = gid0;
			send_msg.vector[2].iov_len = strlen(gid0);

			size_t i;

			for (i = 0; i < msg.vector_size; i++) {
				send_msg.vector[i + 3].iov_base = msg.vector[i].iov_base;
				send_msg.vector[i + 3].iov_len = msg.vector[i].iov_len;
			}

			send_app_msg(&send_msg);
		}
	}
}

int main(int argc, char **argv)
{
	create_io();

	pthread_t       tid_recv;
	int             ret = 0;
	ret = pthread_create(&tid_recv, NULL, (void *)t_msg_recv, NULL);

	if (ret != 0) {
		printf("Create send pthread error!\n");
		return -1;
	}

	load_supex_args(&g_smart_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_call_set;

	smart_mount(&g_smart_cfg_list);
	smart_start();

	return 0;
}

