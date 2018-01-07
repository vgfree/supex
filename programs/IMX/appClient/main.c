/*
 * CopyRight    : DT+
 * Author       : huiqi.qian
 * Date         : 07-15-2017
 * Description  : Appclient
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

#include "json.h"
#include "libkv.h"
#include "libmini.h"
#include "comm_api.h"
#include "libevcs.h"

#define SERVER_NAME "AK47"

int g_sfd = 0;
struct comm_context *g_commctx = NULL;
struct evt_task *g_etask = NULL;

int app_lua_task_awake(lua_State *L)
{
	evt_task_awake(g_etask);
	return 0;
}

int app_lua_task_sleep(lua_State *L)
{
	evt_task_sleep(g_etask);
	return 0;
}

int app_lua_recv_message(lua_State *L)
{
	struct comm_message     recvmsg = { 0 };
	do {
		commmsg_make(&recvmsg, 1024);//FIXME
		int err = commapi_recv(g_commctx, &recvmsg);

		if (err) {
			loger("comm_recv failed\n");
			commmsg_free(&recvmsg);
			continue;
		}
		if (recvmsg.package.packages != 1) {
			loger("comm_recv errmsg\n");
			commmsg_free(&recvmsg);
			continue;
		}
		break;
	} while (1) ;

	loger("\x1B[1;31m" "message fd:%d message body:%.*s socket_type:%d\n" "\x1B[m",
			recvmsg.fd, (int)recvmsg.package.raw_data.len, recvmsg.package.raw_data.str, recvmsg.ptype);

	lua_newtable(L);

	int i = 0;
	for (; i < recvmsg.package.frames_of_package[0]; i++) {
		//printf("frame %d data :%s\n", i + 1, commmsg_frame_addr(&recvmsg, i));
		//printf("frame %d size :%d\n", i + 1, commmsg_frame_size(&recvmsg, i));

		lua_pushnumber(L, i + 1);
		lua_pushlstring(L, commmsg_frame_addr(&recvmsg, i), commmsg_frame_size(&recvmsg, i));
		lua_settable(L, -3);
	}

	commmsg_free(&recvmsg);
	return 1;
}

int app_lua_send_message(lua_State *L)
{
	if (lua_istable(L, -1) == 0) {
		lua_pushboolean(L, 0);
		return 1;
	}

	struct comm_message     sendmsg = { 0 };
	commmsg_make(&sendmsg, 1024);
	commmsg_sets(&sendmsg, g_sfd, 0, PUSH_METHOD);

	int             i;
	size_t          size = 0;
	const char      *data = NULL;
	int             nub = lua_objlen(L, 1);

	for (i = 1; i <= nub; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		data = luaL_checklstring(L, -1, &size);

		/*send fram*/
		commmsg_frame_set(&sendmsg, i - 1, size, data);
		sendmsg.package.frames_of_package[0] = sendmsg.package.frames;
		sendmsg.package.packages = 1;

		/*do next*/
		lua_pop(L, 1);
	}
	lua_pushboolean(L, 1);

	int err = commapi_send(g_commctx, &sendmsg);
	if (err) {
		loger("comm_send failed\n");
	}
	commmsg_free(&sendmsg);

	return 1;
}

static lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);

	{
		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, SERVER_NAME);
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);
		lua_register(L, "app_lua_task_awake", app_lua_task_awake);
		lua_register(L, "app_lua_task_sleep", app_lua_task_sleep);
		lua_register(L, "app_lua_recv_message", app_lua_recv_message);
		lua_register(L, "app_lua_send_message", app_lua_send_message);

		error = luaL_dofile(L, "init.lua");

		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "start.lua");

	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	lua_getglobal(L, "app_init");
	lua_pcall(L, 0, 0, 0);
	return L;
}


static void dispose_request(lua_State *L)
{
	lua_getglobal(L, "app_hand_ask");
	int error = lua_pcall(L, 0, 0, 0);
	if (error) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void dispose_message(lua_State *L)
{
	lua_getglobal(L, "app_hand_msg");
	int error = lua_pcall(L, 0, 0, 0);
	if (error) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void *_pull_thread(void *usr)
{
	lua_State       *L = lua_vm_init();

	dispose_message(L);
	return NULL;
}

static void event_fun(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	switch (step)
	{
		case STEP_INIT:
			printf("client here is connect : %d\n", socket);
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;


		case STEP_STOP:
			printf("client here is close : %d\n", socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}


int main(int argc, char *argv[])
{
	char    *host = "127.0.0.1";
	char    *port = "5000";
	x_printf(D, "nodeServer:%s, nodePort:%s.", host, port);

	g_etask = evt_task_init();
	/*create ctx*/
	g_commctx = commapi_ctx_create();

	if (unlikely(!g_commctx)) {
		loger("client comm_ctx_create failed\n");
		return -1;
	}

	/* 设置回调函数的相关信息 */
	struct comm_cbinfo cbinfo = { 0 };
	cbinfo.fcb = event_fun;
	cbinfo.usr = NULL;


	g_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);
	if (unlikely(g_sfd == -1)) {
		commapi_ctx_destroy(g_commctx);
		loger("client comm_socket failed\n");
		return -1;
	}

	/*network*/
	pthread_t tid;
	int err = pthread_create(&tid, NULL, _pull_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}


	lua_State       *L = lua_vm_init();
	dispose_request(L);

	/*over*/
	loger("goint to detroy everything here\n");
	commapi_ctx_destroy(g_commctx);

	evt_task_free(g_etask);
	return -1;
}
