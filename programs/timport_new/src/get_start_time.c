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

#include "common.h"
#include "get_start_time.h"

int get_start_time(int timestamp)
{
	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_gettime.lua");
	lua_getglobal(L, "get_time");

        lua_pushnumber(L, timestamp);

	int iError = lua_pcall(L, 1, 1, 0);
	if (iError) {
		printf("%d\n", lua_tonumber(L, -1));
		lua_close(L);
		exit(0);
	}

	lua_stack(L);

	int start_time = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_close(L);
	
	return start_time;
}

int read_timestamp()
{
	FILE *pfile=fopen("var/start_time.txt","r");
	if (!pfile) {
		printf("can not find start_time.txt\n");
		exit(0);
	}
	fseek(pfile,0,SEEK_END);
	int len=ftell(pfile);
	char pbuf[len+1];
	rewind(pfile);
	fread(pbuf,1,len,pfile);
	pbuf[len]=0;
	fclose(pfile);
	return atoi(pbuf);
}

static void itoa(int i,char *string)
{
        int power;
        int j = i;
        for(power = 1; j>=10; j /= 10)
        power *= 10;
        for(;power > 0; power /= 10)
        {
      		*string++='0'+i/power;
      		i %= power;
        }
        *string='\0';
}

void write_timestamp(int timestamp)
{
	FILE *pfile=fopen("var/start_time.txt","w");
	if (!pfile) {
                printf("can not find start_time.txt\n");
                exit(0);
        }
	char str[25] = {0};
	itoa(timestamp, str);

	fwrite (str, 1, strlen(str), pfile);
	fclose(pfile);
	fflush(pfile); 
}


/*
int main()
{
	//printf("start time = %d\n", get_start_time(1466166100));
	printf("timestamp = %d\n", read_timestamp());
	write_timestamp(1466166100);
	printf("timestamp = %d\n", read_timestamp());
}
*/
