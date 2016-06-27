#include <stdio.h>
#include "common.h"

void lua_stack(lua_State *L)
{
	int     stack_top = lua_gettop(L);	// 获取栈顶的索引值
	int     idx;
	int     t;

	printf("--栈顶(v)(%d)--\n", stack_top);

	// 显示栈中的元素
	for (idx = stack_top; idx > 0; --idx) {
		t = lua_type(L, idx);
		printf("(i:%d) %s(%s)\n", idx, lua_typename(L, t), lua_tostring(L, idx));
	}

	printf("--栈底--\n");
}

