#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <event.h>
#include <unistd.h>
#include <sys/time.h>

#include <assert.h>  
#include "lua.h"  
#include "lualib.h"  
#include "lauxlib.h"
#include <pthread.h>  

static const target_time = 1465722000;

void *get_data_task (void* args) {  
  printf("enter getdata\n");
  lua_State* L;
  L = luaL_newstate();  
  luaL_openlibs(L);  
  luaL_dofile(L, "../move_data.lua"); 
 
  lua_getglobal(L, "handle");
  lua_pushnumber(L, 1);
 
  int iError = lua_pcall(L, 1, 0, 0);  
  if (iError)  
  {  
    lua_close(L);
    printf("lua_pcall failed\n");  
    exit(0);  
  } 
 
  lua_pop(L,1);  
  lua_close(L);    
}

void readData()
{
  printf("enter readData\n");
  int err;  
  pthread_t getDataThread;
  assert(pthread_create(&getDataThread, NULL, get_data_task, (void *)1) == 0);
  printf("before pthread_join\n");
  pthread_join(getDataThread, NULL);
  printf("after pthread_join\n");
}



int main() {
  return 0;
} 
