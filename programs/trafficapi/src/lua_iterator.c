/********************
* Author   : dujun
* Date     : 2015-12-05
* Function : Encapsulate c for lua calls the iterator function
*
********************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lua_iterator.h"

int iterator_init(lua_State *L)
{
	if (3 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	int     RRID = luaL_checkint(L, 1);
	int     SGID = luaL_checkint(L, 2);
	int     ICNT = luaL_checkint(L, 3);

	map_iterator_element *ptr = map_seg_iterator_init(RRID, SGID, ICNT);

	lua_pushlightuserdata(L, ptr);
	printf("init_ptr:%p\n", ptr);

	return 1;
}

int iterator_next(lua_State *L)
{
	back_seg out;

	if (1 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_isuserdata(L, 1)) {
		printf("userdata!\n");
	}

	map_iterator_element *ptr = (struct map_iterator_element *)lua_touserdata(L, 1);

	printf("next_ptr:%p\n", ptr);

	back_seg *out_re = map_seg_iterator_next(ptr, &out);

	if (!out_re) {
		lua_pushnil(L);
		return 1;
	}

	printf("length:%ld\n", out_re->ptr_seg->length);
	printf("end_grade:%d\n", out_re->ptr_seg->end_grade);
	printf("end_lon:%f\n", out_re->ptr_seg->end_lon);
	printf("end_lat:%f\n", out_re->ptr_seg->end_lat);

	// 将out_re的值压入栈中
	lua_createtable(L, 8, 0);
	lua_pushnumber(L, 1);
	lua_pushnumber(L, out_re->ptr_seg->length);
	lua_rawset(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, out_re->ptr_seg->end_grade);
	lua_rawset(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, out_re->ptr_seg->end_lon);
	lua_rawset(L, -3);

	lua_pushnumber(L, 4);
	lua_pushnumber(L, out_re->ptr_seg->end_lat);
	lua_rawset(L, -3);

	lua_pushnumber(L, 5);

	if (NULL != out_re->ptr_end_name) {
		lua_pushstring(L, out_re->ptr_end_name);
	} else {
		lua_pushstring(L, "");
	}

	lua_rawset(L, -3);

	lua_pushnumber(L, 6);
	lua_pushnumber(L, out_re->ptr_seg->rrid);
	lua_rawset(L, -3);

	lua_pushnumber(L, 7);
	lua_pushnumber(L, out_re->ptr_seg->sgid);
	lua_rawset(L, -3);

	lua_pushnumber(L, 8);
	lua_pushnumber(L, out_re->ptr_seg->sgid_rt);
	lua_rawset(L, -3);

	return 1;
}

int iterator_destory(lua_State *L)
{
	if (1 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	map_iterator_element *ptr = (struct map_iterator_element *)lua_touserdata(L, 1);

	map_seg_iterator_destory(ptr);

	return 0;
}

int get_SGInfo(lua_State *L)
{
	if (2 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	int     RRID = luaL_checkint(L, 1);
	int     SGID = luaL_checkint(L, 2);

	back_seg out;

	int             ret = map_seg_query(RRID, SGID, &out);
	map_seg_info    *temp = out.ptr_seg;

	if (0 != ret) {
		lua_pushnil(L);
		return 1;
	}

	printf("length:%ld\n", temp->length);
	printf("end_grade:%d\n", temp->end_grade);
	printf("end_lon:%f\n", temp->end_lon);
	printf("end_lat:%f\n", temp->end_lat);
	//	if(NULL != out.ptr_end_name)
	//		printf("end_name:%s\n",out.ptr_end_name);

	lua_createtable(L, 6, 0);
	lua_pushnumber(L, 1);
	lua_pushnumber(L, temp->length);
	lua_rawset(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, temp->end_grade);
	lua_rawset(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, temp->end_lon);
	lua_rawset(L, -3);

	lua_pushnumber(L, 4);
	lua_pushnumber(L, temp->end_lat);
	lua_rawset(L, -3);

	lua_pushnumber(L, 5);

	if (NULL != out.ptr_end_name) {
		lua_pushstring(L, out.ptr_end_name);
	} else {
		lua_pushstring(L, "");
	}

	lua_rawset(L, -3);

	lua_pushnumber(L, 6);
	lua_pushnumber(L, temp->sgid_rt);
	lua_rawset(L, -3);

	return 1;
}

