#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lua_mappoi.h"

int getpoi(lua_State *L)
{
	if ((2 != lua_gettop(L))) {
		lua_pushnil(L);
		return 1;
	}

	unsigned int    rrid = luaL_checkint(L, 1);
	unsigned int    sgid = luaL_checkint(L, 2);
	mappoi_iterator *p_it = mappoi_iterator_init(rrid, sgid, 5);

	if (p_it == NULL) {
		lua_pushnil(L);
		return 1;
	}

	mappoi_poi_buf *out;
	// mappoi_poi_buf buf[10]={};
	lua_newtable(L);
	int count = 0;

	while (mappoi_iterator_next(p_it, &out) == 0) {
		if (out != NULL) {
			//	lua_newtable(L);
			int j = 0;
			count++;

			for (j = 0; j < out->poi_idx; j++) {
				char buf[50] = {};
				lua_newtable(L);
				mappoi_poi *p_poi_j = out->poi + j;
				//	printf("poi_id:%d, poi_type:%d,x:%f,y:%f,sg_id:%d,rr_id:%d\n", p_poi_j->poi_id, p_poi_j->poi_type, p_poi_j->point_x, p_poi_j->point_y, p_poi_j->sg_id,p_poi_j->rr_id);
				lua_pushnumber(L, p_poi_j->poi_type);
				lua_setfield(L, -2, "type");

				lua_pushnumber(L, p_poi_j->poi_id);
				lua_setfield(L, -2, "poiID");

				lua_pushnumber(L, p_poi_j->ang);
				lua_setfield(L, -2, "direction");

				lua_pushnumber(L, p_poi_j->point_y);
				lua_setfield(L, -2, "latitude");

				lua_pushnumber(L, p_poi_j->sg_id);
				lua_setfield(L, -2, "sgid");

				lua_pushnumber(L, p_poi_j->rr_id);
				lua_setfield(L, -2, "rrid");

				lua_pushnumber(L, p_poi_j->point_x);
				lua_setfield(L, -2, "longitude");
				sprintf(buf, "%d", p_poi_j->poi_id);
				lua_setfield(L, -2, buf);
			}
		}
	}

	if (count == 0) {
		mappoi_iterator_destory(p_it);
		lua_pushnil(L);
		return 1;
	} else {
		mappoi_iterator_destory(p_it);
		return 1;
	}
}

