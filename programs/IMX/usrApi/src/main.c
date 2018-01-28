#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "libmini.h"
#include "comm_api.h"
#include "usrapi_i_wrap.h"
#include "usrapi_o_wrap.h"

#define SERVER_NAME "usrApi"

extern int g_manage_sfd;

static int downstream_msg(struct comm_message *msg)
{
	commmsg_sets(msg, g_manage_sfd, 0, PUSH_METHOD);

	if (usrapi_i_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}

	return 0;
}

int app_lua_send_message(lua_State *L)
{
	if (lua_istable(L, -1) == 0) {
		lua_pushboolean(L, 0);
		return 1;
	}

	struct comm_message sendmsg = { 0 };
	commmsg_make(&sendmsg, 1024);

	int             i;
	int             nub = lua_objlen(L, 1);

	for (i = 1; i <= nub; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		size_t          fsize = 0;
		const char      *frame = luaL_checklstring(L, -1, &fsize);

		/*send fram*/
		commmsg_frame_set(&sendmsg, i - 1, fsize, (char *)frame);
		sendmsg.package.frames_of_package[0] = sendmsg.package.frames;
		sendmsg.package.packages = 1;

		/*do next*/
		lua_pop(L, 1);
	}

	lua_pushboolean(L, 1);

	downstream_msg(&sendmsg);

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

static void *_pull_thread(void *usr)
{
	lua_State *L = lua_vm_init();

	while (1) {
		struct comm_message msg = {};
		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		usrapi_o_wrap_recv(&msg);

#define debug 1
#ifdef debug
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
		commmsg_print(&msg);
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
#endif

		lua_getglobal(L, "app_call");
		lua_newtable(L);

		int i = 0;

		for (i = 0; i < commmsg_frame_count(&msg); i++) {
			int     fsize = 0;
			char    *frame = commmsg_frame_get(&msg, i, &fsize);

			lua_pushnumber(L, i + 1);
			lua_pushlstring(L, frame, fsize);
			lua_settable(L, -3);
		}

		commmsg_free(&msg);

		int error = lua_pcall(L, 1, 0, 0);

		if (error) {
			printf("%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}

	return NULL;
}

static pthread_t tid1;
void usrapi_work(void)
{
	assert(usrapi_i_wrap_init() == 0);
	assert(usrapi_o_wrap_init() == 0);

	/*work push*/
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	x_printf(I, "usrApi work!\n");
}

void usrapi_wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
}

void usrapi_stop(void)
{
	x_printf(W, "usrApi stop!\n");
	usrapi_i_wrap_exit();
	usrapi_o_wrap_exit();
}

int main(int argc, char *argv[])
{
	/*init log*/
	SLogOpen("openChat.log", SLogIntegerToLevel(0));

	usrapi_work();

	usrapi_wait();

	usrapi_stop();

	return 0;
}

