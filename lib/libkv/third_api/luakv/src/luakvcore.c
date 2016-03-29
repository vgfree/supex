//
//  luakv.c
//  bLib
//
//  Created by 周凯 on 15/6/16.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#include "luakvcore.h"
#include <ctype.h>

/* ===== INITIALISATION ===== */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source.
 */
        static void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
        {
                int i;

                luaL_checkstack(l, nup, "too many upvalues");

                for (; reg->name != NULL; reg++) {      /* fill the table with given functions */
                        for (i = 0; i < nup; i++) {     /* copy upvalues to the top */
                                lua_pushvalue(l, -nup);
                        }

                        lua_pushcclosure(l, reg->func, nup); /* closure with those upvalues , and pop all of them*/
                        lua_setfield(l, -(nup + 2), reg->name);
                }

                lua_pop(l, nup); /* remove upvalues */
        }
#endif

/*
 * 通过闭包实现迭代器
 * 在栈顶返回nil时，停止遍历
 */
static
int luakv_iterclosure(lua_State *L)
{
        if (lua_type(L, lua_upvalueindex(1)) != LUA_TNIL) {
                /* 可以继续下次遍历 */
        	lua_pushvalue(L, lua_upvalueindex(1));
        	luakv_iternext(L);
        } else {
        	lua_pushnil(L);
        }
        
        return 1;
}

/**
 * 以遍历的方式执行libkv的命令
 * 在栈顶返回一个闭包C函数
 */
int luakv_iterfactory(lua_State *L)
{
        // 获取answer
        //stack top -> bottom : command, handle
        luakv_ask(L);
        //stack top -> bottom : answer, command
        // 找到数据
        if (lua_type(L, -1) != LUA_TNIL) {
                luakv_getiter(L);
        }

        // 当前栈中数据 
        // stack top -> bottom : iter, command
        lua_pushcclosure(L, &luakv_iterclosure, 1);

        return 1;
}

/*
 * 将传入的表组合成单个的命令存入，并在栈顶返回该表
 */
static
int luakv_parseargs(lua_State *L)
{
        int             i = 0;
        const int       startpos = 2;
        int             argc = lua_gettop(L);

        if (argc < startpos) {
                luaL_error(L, "Bad argument, 'value' expected");
        }

        int type = lua_type(L, startpos);

        if (type == LUA_TNIL) {
                luaL_error(L, "Bad argument, 'value' expected");
        }

        // 组合所有的参数
        luaL_Buffer buffer;

        if (type != LUA_TTABLE) {
                luaL_buffinit(L, &buffer);

                for (i = startpos; i < argc + 1; ++i) {
                        /* code */
                        luaL_addstring(&buffer, luaL_checkstring(L, i));
                        luaL_addchar(&buffer, ' ');
                }

                luaL_pushresult(&buffer);
        } else {
                /*must one table*/
                if (argc != startpos) {
                        luaL_error(L, "Bad argument, only one argument expected");
                }

                /*store request table*/
                lua_newtable(L);
                luaL_newmetatable(L, ".parse.command");
                lua_pushstring(L, "kv");
                lua_setfield(L, -2, "__mode");
                lua_setmetatable(L, -2);
#ifdef __APPLE__
                        lua_pushstring(L, "@@");
                        lua_setfield(L, -2, "@@");
#endif

                //传入的表必须是{ {'set', 'key', 'value' }, {'set', 'key', 'value' } }的格式
                for (i = 1;; i++) {
                        int j = 0;

                        lua_rawgeti(L, -2, i);
                        /*is over*/
                        type = lua_type(L, -1);

                        if (type == LUA_TNIL) {
                                lua_pop(L, 1);
                                break;
                        }

                        /*is next*/
                        if (type != LUA_TTABLE) {
                                luaL_error(L, "Bad argument, 'array table' expected");
                        }

                        // 遍历表中的表
                        luaL_buffinit(L, &buffer);

                        for (j = 1;; j++) {
                                lua_rawgeti(L, -1, j);

                                if (lua_type(L, -1) == LUA_TNIL) {
                                        lua_pop(L, 1);
                                        break;
                                }

                                luaL_addstring(&buffer, luaL_checkstring(L, -1));
                                luaL_addchar(&buffer, ' ');
                                lua_pop(L, 1);
                        }

                        /*current table*/
                        lua_pop(L, 1);

                        if (j != 1) {
                                luaL_pushresult(&buffer);
                                lua_rawseti(L, -2, i);
                        } else {
                                luaL_error(L, "Bad argument, 'array table' is empty.");
                        }
                }

                //传入的是空表
                if (i == 1) {
                        luaL_error(L, "Bad argument, 'array table' is empty.");
                }

#ifdef __APPLE__
                        lua_pushnil(L);
                        lua_setfield(L, -2, "@@");
#endif
        }

        // pop all arguments, and remain handle and command-table only
        lua_insert(L, startpos);
        lua_settop(L, startpos);

        return 2;
}

/*
 * 根据libkv的redis命令进行结果转换
 * 将栈顶的表中的结果转换为每个命令应对应的结果（字符串、数字、表）
 * 如果发生了转换，返回值放置栈顶并将原有的表弹出栈
 */
static
int luakv_transformresult(lua_State *L)
{
        int             i = 0;
        int             start = -1;
        int             end = -1;
        size_t          len = 0;
        const char      *command = luaL_checklstring(L, -2, &len);

        // which command of redis
        for (i = 0; i < (int)len; i++) {
                if ((start == -1) && !isspace(command[i])) {
                        start = i;
                }

                if ((start != -1) && (end == -1) && isspace(command[i])) {
                        /* code */
                        end = i;
                        break;
                }
        }

        if (end == -1) {
                end = (int)len;
        }

        if ((start == -1) || (end <= start)) {
                luaL_error(L, "%s", "Can't get operator which redis command.");
        }

        // fprintf(stderr, "redis command : %.*s\n", end - start, command + start);

        if ((strncasecmp("sadd", command + start, end - start) == 0) ||
                (strncasecmp("del", command + start, end - start) == 0) ||
                (strncasecmp("dbsize", command + start, end - start) == 0) ||
                (strncasecmp("inr", command + start, end - start) == 0) ||
                (strncasecmp("inrby", command + start, end - start) == 0) ||
                (strncasecmp("decr", command + start, end - start) == 0) ||
                (strncasecmp("decrby", command + start, end - start) == 0) ||
                (strncasecmp("lpush", command + start, end - start) == 0) ||
                (strncasecmp("expire", command + start, end - start) == 0) ||
                (strncasecmp("expireat", command + start, end - start) == 0) ||
                (strncasecmp("pexpire", command + start, end - start) == 0) ||
                (strncasecmp("pexpireat", command + start, end - start) == 0) ||
                (strncasecmp("len", command + start, end - start) == 0) ||
                (strncasecmp("rpush", command + start, end - start) == 0) ||
                (strncasecmp("hset", command + start, end - start) == 0) ||
                (strncasecmp("sismember", command + start, end - start) == 0) ||
                (strncasecmp("scard", command + start, end - start) == 0)) {
                /* 返回一个数值 */
                // fprintf(stderr, "%s\n", "number @@@@@@");
                lua_Number result = 0;
        	// stack top -> bottom : table command
                lua_rawgeti(L, -1, 1);

                // stack top -> bottom : result table command
                if (lua_type(L, -1) != LUA_TNIL) {
                        result = lua_tonumber(L, -1);
                }
                // stack top -> bottom : result table command
                lua_pop(L, 2);
                // stack top -> bottom : command
                lua_pushnumber(L, result);
                // stack top -> bottom : result command
        } else if ((strncasecmp("set", command + start, end - start) == 0) ||
                (strncasecmp("flushdb", command + start, end - start) == 0) ||
                (strncasecmp("exists", command + start, end - start) == 0) ||
                (strncasecmp("mset", command + start, end - start) == 0) ||
                (strncasecmp("select", command + start, end - start) == 0) ||
                (strncasecmp("flushall", command + start, end - start) == 0) ||
                (strncasecmp("hmset", command + start, end - start) == 0)) {
        	/*返回一个布尔值*/
                // fprintf(stderr, "%s\n", "boolean @@@@@@");
                int b = 0;
                // stack top -> bottom : table command
                lua_rawgeti(L, -1, 1);

                // stack top -> bottom : result table command
                if (lua_type(L, -1) != LUA_TNIL) {
                        b = 1;
                } else {
                	b = 0;
                }
                // stack top -> bottom : result table command
                lua_pop(L, 2);
                // stack top -> bottom : command
                lua_pushboolean(L, b);
                // stack top -> bottom : result command
        } else if ((strncasecmp("get", command + start, end - start) == 0) ||
        	(strncasecmp("echo", command + start, end - start) == 0) ||
        	(strncasecmp("rpop", command + start, end - start) == 0) ||
        	(strncasecmp("lpop", command + start, end - start) == 0) ||
        	(strncasecmp("type", command + start, end - start) == 0) ||
        	(strncasecmp("hget", command + start, end - start) == 0)) {
        	/*返回一个字符串*/
                // fprintf(stderr, "%s\n", "first @@@@@@");
                // stack top -> bottom : table command
                lua_rawgeti(L, -1, 1);
                // stack top -> bottom : result table command
                lua_insert(L, -2);
                // stack top -> bottom : table result command
                lua_pop(L, 1);
                // stack top -> bottom : result command
        } else {
                // no transform
                // lrange
                // smembers
                // hmget
                // srandmember
        }

        //移除命令参数
        // stack top -> bottom : result command
        lua_remove(L, -2);
        // stack top -> bottom : result
        return 1;
}

/*
 * 执行一条libkv的命令
 * 在栈顶返回命令对应的数据
 */
static
int luakv_exec(lua_State *L)
{
        int i = 0;

        luakv_ask(L);

        // stack top -> bottom : answer/nil command
        lua_newtable(L);
        // stack top -> bottom : table answer/nil command

        luaL_newmetatable(L, ".store.result.single-command.to.table");
        lua_pushstring(L, "kv");
        lua_setfield(L, -2, "__mode");
        lua_setmetatable(L, -2);
#ifdef __APPLE__
                lua_pushstring(L, "@@");
                lua_setfield(L, -2, "@@");
#endif

        // stack top -> bottom : table answer/nil command
        lua_insert(L, -2);
        // stack top -> bottom : answer/nil table command

        /*
         * 判断命令执行后，是否有结果数据需要转存到表
         */
        if (lua_type(L, -1) != LUA_TNIL) {
        	//获取适配器
        	// stack top -> bottom : answer table command
                luakv_getiter(L);
                
                while (1) {

                	// stack top -> bottom : iterator table command
                        luakv_iternext(L);

                        if (lua_type(L, -1) == LUA_TNIL) {
                                // stact top -> bottom : value table command
                                lua_pop(L, 1);
                                break;
                        } else {
                        	// stact top -> bottom : value iterator table command
                        	lua_rawseti(L, -3, ++i);
                        	// stact top -> bottom : iterator table command
                        }
                }


        } else {
        	// stack top -> bottom : nil table command
                lua_pop(L, 1);
        }

#ifdef __APPLE__
                lua_pushnil(L);
                lua_setfield(L, -2, "@@");
#endif

        // 对结果进行转换
        // stack top -> bottom : table command
        luakv_transformresult(L);
        // stack top -> bottom : result(string/number/table)

        return 1;
}

/**
 * 执行命令集合，在栈顶返回命令结果
 * luakv_run(handle, {{'set', 'key', 'value'}, {'del', 'key'}})
 */
int luakv_run(lua_State *L)
{
        // 解析命令
        // stack top -> bottom : commandtable/commandstring handle
        luakv_parseargs(L);

        if (lua_type(L, -1) == LUA_TSTRING) {
                /*one task*/
                // stack top -> bottom : commandtable/commandstring handle
                luakv_exec(L);
                // stack top -> bottom : result
        } else {
                /*more task*/
                int i = 0;

                /*store result table*/
                lua_newtable(L);
                luaL_newmetatable(L, ".store.multi-command.result.to.table");
                lua_pushstring(L, "kv");
                lua_setfield(L, -2, "__mode");
                lua_setmetatable(L, -2);
#ifdef __APPLE__
                        lua_pushstring(L, "@@");
                        lua_setfield(L, -2, "@@");
#endif

                // stack top -> bottom : resulttable commandtable handle
                lua_insert(L, 1);

                // 遍历命令表，挨个执行
                // stack top -> bottom : commandtable handle resulttable
                lua_pushnil(L);
                // stack top -> bottom : index commandtable handle resulttable
                while (lua_next(L, -2) != 0) {
                        if (lua_type(L, -2) == LUA_TNUMBER) {
                                // stack top -> bottom : command index commandtable handle resulttable
                                lua_pushvalue(L, 2);
                                // stack top -> bottom : handle command index commandtable handle resulttable
                                lua_insert(L, -2);
                                // stack top -> bottom : command handle index commandtable handle resulttable
                                luakv_exec(L);
                                // stack top -> bottom : result index commandtable handle resulttable
                                lua_rawseti(L, 1, ++i);
                                // stack top -> bottom : index commandtable handle resulttable
                        } else {
                                lua_pop(L, 1);
                        }
                        // stack top -> bottom : index commandtable handle resulttable
                }

                // stack top -> bottom : commandtable handle resulttable
                lua_pop(L, 2);

                // stack top -> bottom : resulttable
#ifdef __APPLE__
                        lua_pushnil(L);
                        lua_setfield(L, 1, "@@");
#endif
        }

        return 1;
}

static
const luaL_Reg regLib[] = {
        { "createbyptr", luakv_createbyptr  },
        { "create", luakv_create            },
        { "run",    luakv_run               },
        { "ask",    luakv_iterfactory       },
        { NULL,     NULL                    }
};

int luaopen_luakv(lua_State *L)
{
        lua_newtable(L);
        luaL_setfuncs(L, regLib, 0);

        return 1;
}