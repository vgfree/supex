#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include <assert.h>  
#include "lua.h"  
#include "lualib.h"  
#include "lauxlib.h"

#include "dispatch_data.h"

static void stackDump(lua_State* L)
{
	printf("begin dump lua stack\n");
	int i = 0;
	int top = lua_gettop(L);
	for (i = 1; i <= top; ++i) {
		int t = lua_type(L, i);
        	switch (t) {
            		case LUA_TSTRING:
                	printf("'%s' ", lua_tostring(L, i));
                	break;
            		case LUA_TBOOLEAN:
                	printf(lua_toboolean(L, i) ? "true " : "false ");
			break;
            		case LUA_TNUMBER:
                	printf("%g ", lua_tonumber(L, i));
                	break;
            		default:
                	printf("%s ", lua_typename(L, t));
                	break;
        	}
	}
    	printf("end dump lua stack\n");
}

int dispatchToUser (char *user, int userLen, char * time, int timeLen) {  
  	lua_State* L;
  	L = luaL_newstate();  
  	luaL_openlibs(L);  
  	luaL_dofile(L, "./timport_server.lua"); 
  	lua_getglobal(L, "GetTable");
	lua_newtable(L);
	//stackDump(L);

 	lua_pushnumber(L, 1);
	lua_pushlstring(L, user, userLen);
	lua_settable(L, -3);
	//stackDump(L);

	lua_pushnumber(L, 2);
        lua_pushlstring(L, time, timeLen);
        lua_settable(L, -3);
	//stackDump(L);
	
  	int iError = lua_pcall(L, 1, 0, 0);  
  	if (iError)  
  	{ 
		printf("%s\n", lua_tostring(L, -1)); 
    		lua_close(L);  
    		exit(0);  
  	} 
 
  	lua_pop(L,1);  
  	lua_close(L);    
 }
/*
int main() {
	char * name = "908097901013119";
	char * time = "20160616104";
	dispatchToUser(name, strlen(name), time, strlen(time));
	return 0;
}
*/
