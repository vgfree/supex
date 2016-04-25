#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "loadgps.h"
#include "sg_info.h"

static bool popTable(lua_State* L, int idx, sg_png_manag_t *sg)
{
        sg_info_t temp_info;
        const char*  value;
        lua_pushnil(L);
        while(lua_next(L, idx) != 0){
                int keyType = lua_type(L, -2);
                if(keyType == LUA_TNUMBER){
                        double value = lua_tonumber(L, -2);
                        //printf("key:%d\n", value); 
                }else if(keyType == LUA_TSTRING){
                        value = lua_tostring(L, -2);
                        //printf("key:%s\n", value); 
                }else{
                        printf("Invalid key typ:%d\n", keyType);
                        return false;
                }                                                                                           
                int valueType = lua_type(L, -1);
                switch(valueType){
                        case LUA_TNIL:
                                {
                                        printf("Value: nil\n");
                                        break;
                                }
                        case LUA_TBOOLEAN:
                                {
                                        int value = lua_toboolean(L, -1);
                                        //printf("%d\n", value);
                                        break;
                                }
                        case LUA_TNUMBER:
                                {
                                        if(strncmp(value, "TT", 2) == 0) {
                                                //printf("TT Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.tt = lua_tonumber(L, -1);
                                                //printf("tt:%d\n", temp_info.tt);
                                                sg_png_manage_add(sg, &temp_info);
                                        }
                                        else if(strncmp(value, "SGSB", 4) == 0) {
                                                //printf("SGSB Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.SB = lua_tonumber(L, -1);
                                                //printf("SGSB:%lf\n", temp_info.SB);
                                        }
                                        else if(strncmp(value, "SGSL", 4) == 0) {
                                                //printf("SGSL Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.SL = lua_tonumber(L, -1);
                                                //printf("SGSL:%lf\n", temp_info.SL);
                                        }
                                        else if(strncmp(value, "SGEB", 4) == 0) {
                                                //printf("SGEB Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.EB = lua_tonumber(L, -1);
                                                //printf("SGEB:%lf\n", temp_info.EB);
                                        }
                                        else if(strncmp(value, "RT", 2) == 0) {
                                                //printf("RT Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.rt = lua_tonumber(L, -1);
                                                //printf("Rt:%lf\n", temp_info.rt);
                                        }
                                        else {
                                                //printf("SGEL Value:%lf\n", lua_tonumber(L, -1));
                                                temp_info.EL = lua_tonumber(L, -1);
                                                //printf("SGEL:%lf\n", temp_info.EL);
                                        }
                                        break;
                                }
                        case LUA_TSTRING:
                                {
                                        if(strncmp(value, "NAME", 4) == 0) {
                                                char *name = lua_tostring(L, -1);
                                                //printf("NAME Value:%s\n", name);
                                                strcpy(temp_info.name, name);
                                                //printf("name:%s\n", temp_info.name);
                                        }
                                        else if(strncmp(value, "SGID", 4) == 0) {
                                                char *name = lua_tostring(L, -1);
                                                //printf("SGID Value:%s\n", name);
                                                temp_info.sgid = atoi(name);
                                                //printf("SGID:%d\n", temp_info.sgid);
                                        }
                                        else {
                                                char *name = lua_tostring(L, -1);
                                                //printf("RRID Value:%s\n", name);
                                                temp_info.rrid = atoi(name);
                                                //printf("RRID:%d\n", temp_info.rrid);
                                        }
                                        break;
                                }
                        case LUA_TTABLE:
                                {

                                        //printf("====sub table===\n");
                                        int index = lua_gettop(L);
                                        if (!popTable(L, index, sg)) {
                                                printf("popTable error in  popTable,error occured\n");
                                                return false;
                                        }
                                        break;
                                }
                        default:
                                {
                                        printf("Invalid value type:%d\n", valueType);
                                        return false;
                                }
                }
                lua_pop(L, 1);
        }
        return true;
}

int loadgps_form_tablefile(char *filename, char *tablename, sg_png_manag_t *sg)
{
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        int r = luaL_dofile(L, filename);
        lua_getglobal(L, tablename);
        int type = lua_type(L,1);
        if(type == LUA_TTABLE){
                int index = lua_gettop(L);
                if(popTable(L,index,sg)){
                        return 0;
                }else{
                        printf("error\n");
                        return -1;
                }
        } 
        return 0;
}
#if 0
int main(int argc, char *argv[])
{
        loadgps_form_tablefile(argv[1], argv[2]);
        return 0;
}
#endif
