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

void *work_task (void* args) {  
  lua_State* L;
  L = luaL_newstate();  
  luaL_openlibs(L);  
  luaL_dofile(L, "test.lua");  
  int iError = lua_pcall(L, 0, 0, 0);  
  if (iError)  
  {  
    lua_close(L);  
    exit(0);  
  } 
 
  lua_pop(L,1);  
  lua_close(L);    
 }

void *process_start(void *(*fcb)(void *args), void *args)
{
        pid_t pid = 0;

        if ((pid = fork()) < 0) {
                perror("fork");
                exit(-1);
        } else if (pid == 0) {
                fcb(args);
                exit(0);
        } else {
                printf("FORK ONE PROCESS -->PID :%d\n", pid);
        }

        return NULL;
}

int main() {
  for (int i = 0; i < 100; i++)	{
    process_start(work_task, (void *)(i));
  }
  return 0;
} 
