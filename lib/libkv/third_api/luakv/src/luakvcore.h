/*
 *  luakvcore.h
 *  bLib
 *
 *  Created by 周凯 on 15/6/16.
 *  Copyright (c) 2015年 zk. All rights reserved.
 */

#ifndef __luakvcore__h__
#define __luakvcore__h__

#include "luakvutils.h"

__BEGIN_DECLS
/*
 * 使用字符串执行libkv命令
 * lua脚本调用示例：luakv_iterfactory('set name tom')
 * 返回一个lua的闭合函数，可用于for ... in ... do ... end中。
 * 对闭合函数调用将返回结果
 */
int luakv_iterfactory(lua_State *L);


/*
 * 使用变参和表组合的变参执行libkv命令
 * lua脚本调用示例：
 * 使用变参调用：luakv_run('set', 'name', 'tom')，返回命令对应的结果，如果命令返回是集合结果，则以lua的表表示，未查询到数据用nil或空表表示
 * 使用表组合变参调用：luakv_run({ { 'set', 'name', 'tom' }, { 'get', 'name' } })，返回表，表中的元素对应每条命令的结果（其表现形式见上）。
 * 请使用 for i = 1, table.maxn(result) do ... end 这样的形式遍历结果，以防止nil值中断遍历
 */
int luakv_run(lua_State *L);

__END_DECLS

#endif
