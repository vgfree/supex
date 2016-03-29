#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "match_road.h"

int check_parameter(double lon, double lat, short dir)
{
	if ((lon > 180.0) || (lon < -180.0)) {
		return -1;
	}

	if ((lat > 90.0) || (lat < -90.0)) {
		return -1;
	}

	if ((dir > 360.0) || (dir < -1.0)) {
		return -1;
	}

	return 0;
}

int entry_cmd_locate(lua_State *L)
{
	if (3 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	map_line_info   *line;
	double          longitude = (double)luaL_checknumber(L, 1);
	double          latitude = (double)luaL_checknumber(L, 2);
	short           direction = (short)luaL_checknumber(L, 3);

	x_printf(I, "lon:%f, lat:%f, dir:%d\n", longitude, latitude, direction);

	int check = check_parameter(longitude, latitude, direction);

	if (0 != check) {
		lua_pushnil(L);
		return 0;
	}

	int ret = pmr_locate(&line, direction, longitude, latitude);

	if (0 != ret) {
		lua_pushnil(L);
		return 0;
	}

	lua_newtable(L);

	lua_pushstring(L, "RRID");
	lua_pushnumber(L, line->rr_id);
	lua_settable(L, -3);

	lua_pushstring(L, "SGID");
	lua_pushnumber(L, line->sgid);
	lua_settable(L, -3);

	lua_pushstring(L, "trafficID");
	lua_pushnumber(L, line->tfid);
	lua_settable(L, -3);

	lua_pushstring(L, "countyCode");
	lua_pushnumber(L, line->countyCode);
	lua_settable(L, -3);

	lua_pushstring(L, "RT");
	lua_pushnumber(L, line->rt);
	lua_settable(L, -3);

	return 1;
}

