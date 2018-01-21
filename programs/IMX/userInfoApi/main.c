#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "skt.h"

#define SERVER_NAME "AK47"

#if 0
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
#else
int app_lua_send_message(lua_State *L)
{
	if (lua_istable(L, -1) == 0) {
		lua_pushboolean(L, 0);
		return 1;
	}

printf("--------------\n");
	int             i;
	int             nub = lua_objlen(L, 1);
	for (i = 1; i <= nub; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);

		size_t          fsize = 0;
		const char      *frame = luaL_checklstring(L, -1, &fsize);

		/*send fram*/
		zmq_msg_t       msg_frame;
		int             rc = zmq_msg_init_size(&msg_frame, fsize);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsize);
printf("%s\n", frame);
		zmq_cli_notify(&msg_frame, (i == nub) ? 0 : ZMQ_SNDMORE);

		/*do next*/
		lua_pop(L, 1);
	}
	lua_pushboolean(L, 1);
printf("--------------\n");

	return 1;
}
#endif


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
		lua_register(L, "app_lua_send_message", app_lua_send_message);

		error = luaL_dofile(L, "lua/init.lua");

		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "lua/start.lua");

	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	lua_getglobal(L, "app_init");
	lua_pcall(L, 0, 0, 0);
	return L;
}

void *work_task(void *args)
{
	int             error = 0;
	void            *data = NULL;
	void            *g_subscriber = NULL;
	lua_State       *L = lua_vm_init();

	struct skt_device devc = {};

	while (1) {
		zmq_srv_fetch(&devc);

		lua_getglobal(L, "app_call");
		lua_newtable(L);
		int i = 0;
		printf("cnt %d\n", devc.idx);

		for (i = 0; i < devc.idx; i++) {
			data = devc.ibuffer[i].iov_base;
			lua_pushnumber(L, i + 1);
			lua_pushlstring(L, data, devc.ibuffer[i].iov_len);
			lua_settable(L, -3);

			free(data);
		}

		error = lua_pcall(L, 1, 0, 0);

		if (error) {
			printf("%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
}

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	zmq_srv_init("127.0.0.1", 9000);
	zmq_cli_init("127.0.0.1", 10000);
#ifdef SELECT_MULTITHREAD
	zmq_threadstart((zmq_thread_fn *)work_task, (void *)1);
#else
	zmq_process_start((zmq_thread_fn *)work_task, (void *)1);
#endif

	zmq_srv_start();

	// sleep(100);
	zmq_cli_exit();
	zmq_srv_exit();
}

