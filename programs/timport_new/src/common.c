#include <stdio.h>
#include "common.h"

void printLuaStack(lua_State *L)
{
	int stackTop=lua_gettop(L);//获取栈顶的索引值
	int index,t;
	printf("--栈顶(v)(%d)--\n",stackTop);
	//显示栈中的元素
	for(index=stackTop;index>0;--index)
	{
		t=lua_type(L,index);
		printf("(i:%d) %s(%s)\n",index,lua_typename(L,t),lua_tostring(L,index));
        }
	printf("--栈底--\n");
}

