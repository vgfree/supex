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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "load_cfg.h"
#include "major/smart_api.h"
#include "appsrv.h"
#include "json.h"

struct smart_cfg_list g_smart_cfg_list = {};

char load_cid[256] = {};

static void stackDump (lua_State *L);
static int get_uid(lua_State *L);
static int set_uidmap(lua_State *L);
static int send_msg(lua_State *L);
int luaopen_power(lua_State *L);
void *t_msg_recv();
int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int main(int argc, char **argv)
{
	create_io();

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

int luaopen_power(lua_State *L)
{
	lua_register(L, "get_uid", get_uid);
	lua_register(L, "set_uidmap", set_uidmap);
	lua_register(L, "send_msg",  send_msg);

	return 0;
}

static int get_uid(lua_State *L)
{
	printf("下发数据: 获取uid\n");

	char *cid_str = lua_tostring(L, -1);

	char *downstream = "downstream";
	char *cid = "cid";
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

	printf("获取uid完成\n\n");

	return 1;
}

static int set_uidmap(lua_State *L)
{
	printf("下发数据: 绑定uidmap\n");

	int n = lua_gettop(L);
	printf("stack number is %d\n",n);

	lua_pushnumber(L, 1);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_1 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 2);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_2 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 3);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_3 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 4);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_4 = lua_tostring(L, -1);
	lua_pop(L, 1);

	struct app_msg  send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = frame_1;
	send_msg.vector[0].iov_len = strlen(frame_1);
	send_msg.vector[1].iov_base = frame_2;
	send_msg.vector[1].iov_len = strlen(frame_2);
	send_msg.vector[2].iov_base = frame_3;
	send_msg.vector[2].iov_len = strlen(frame_3);
	send_msg.vector[3].iov_base = frame_4;
	send_msg.vector[3].iov_len = strlen(frame_4);
	send_app_msg(&send_msg);
	printf("绑定完成\n\n");

	return 1;
}

static int send_msg(lua_State *L)
{
	printf("下发消息\n");

	int n = lua_gettop(L);

	lua_pushnumber(L, 1);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_1 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 2);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_2 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 3);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_3 = lua_tostring(L, -1);
	lua_pop(L, 1);

	struct app_msg  send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = frame_1;
	send_msg.vector[0].iov_len = strlen(frame_1);
	send_msg.vector[1].iov_base = frame_2;
	send_msg.vector[1].iov_len = strlen(frame_2);
	send_msg.vector[2].iov_base = frame_3;
	send_msg.vector[2].iov_len = strlen(frame_3);
	printf("消息发送完成\n\n");

	return 1;

}

void *t_msg_recv()
{

	while (1) {
		struct app_msg recv_msg = {};
		int more = 0;
		recv_app_msg(&recv_msg, &more, -1);
		int i;
		for (i = 0; i <= recv_msg.vector_size; i++) {
			printf("recv_msg size:%d\t [%d]iov_len:%d\t [%d]iov_base = %s,\n",
					recv_msg.vector_size, i, recv_msg.vector[i].iov_len, i , recv_msg.vector[i].iov_base);
		}

		char str[32];
		strcpy(str, recv_msg.vector[0].iov_base);
		int len = strlen(str);

		printf("len = %d\t第一帧 = %s\n", len, str);

		{
			lua_State *L;
			L = luaL_newstate(); // 打开lua
			luaL_openlibs(L); // 打开标准库

			luaopen_power(L);

			int status = luaL_loadfile(L, "script.lua");
			if (status) {
				perror("luaL_dofile error");
				exit(1);
			}

			lua_newtable(L);

			for (i = 1; i <= recv_msg.vector_size; i++) {
				printf("recv_msg size:%d\t [%d]iov_len:%d\t [%d]iov_base = %s,\n",
						recv_msg.vector_size, i - 1, recv_msg.vector[i - 1].iov_len, i -1 , recv_msg.vector[i - 1].iov_base);
				lua_pushnumber(L, i);
				lua_pushlstring(L, recv_msg.vector[i - 1].iov_base, recv_msg.vector[i - 1].iov_len);
				lua_rawset(L, -3);
			}
			printf("\n");
			// 声明lua中的变量
			lua_setglobal(L, "msg");
			//lua_call(L, 0, 0);
			int result = lua_pcall(L, 0, LUA_MULTRET, 0);
			if (result) {
				fprintf(stdout, "bad, bad script\n");
				exit(1);
			}    /* 获得堆栈顶的值*/
			int sum = lua_tonumber(L, lua_gettop(L));
			if (!sum) {
				fprintf(stdout, "lua_tonumber() failed!\n");
				exit(1);
			}
			fprintf(stdout, "Script returned: %d\n", sum);
			lua_pop(L, 1);
			printf("top = %d\n", lua_gettop(L));
			lua_close(L);
			printf("\n");
		}
	}
}

int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	char msg[64];

	struct data_node *p_node = get_pool_data(task->sfd);
	char *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	printf("p_buf = %s\n", p_buf);

	struct redis_status *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	printf("len: %d \n", p_rst->field[1].len);
	printf("command_type: %x \n", p_rst->command_type);

	memcpy(msg, p_buf + p_rst->field[1].offset, MIN(p_rst->field[1].len, 64 - 1));
	struct app_msg  send_msg = {};
	send_app_msg(&send_msg);

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




