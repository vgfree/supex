//
//  lualog_utils.c
//  bLib
//
//  Created by 周凯 on 15/8/25.
//  Copyright (c) 2015年 zk. All rights reserved.
//
#ifdef _GNU_SOURCE
  #include <dlfcn.h>
#endif
#include <execinfo.h>
#include "lualog_utils.h"

/* -----------          */

/* macro to `unsign' a character */
#undef uchar
#define uchar(c) ((unsigned char)(c))

#undef L_ESC
#define L_ESC '%'

/*
 * 模拟 string.format()
 */
/* maximum size of each formatted item (> len(format('%99.99f', -1e308))) */
#undef MAX_ITEM
#define MAX_ITEM        512
/* valid flags in a format specification */
#undef FLAGS
#define FLAGS           "-+ #0"

/*
** maximum size of each format specification (such as '%-099.99d')
** (+10 accounts for %99.99x plus margin of error)
*/
#undef MAX_FORMAT
#define MAX_FORMAT (sizeof(FLAGS) + sizeof(LUA_INTFRMLEN) + 10)

static
void _addquoted(lua_State *L, luaL_Buffer *b, int arg)
{
	size_t          l;
	const char      *s = luaL_checklstring(L, arg, &l);

	luaL_addchar(b, '"');

	while (l--) {
		switch (*s)
		{
			case '"':
			case '\\':
			case '\n':
			{
				luaL_addchar(b, '\\');
				luaL_addchar(b, *s);
				break;
			}

			case '\0':
			{
				luaL_addlstring(b, "\\000", 4);
				break;
			}

			default:
			{
				luaL_addchar(b, *s);
				break;
			}
		}
		s++;
	}

	luaL_addchar(b, '"');
}

static
const char *_scanformat(lua_State *L, const char *strfrmt, char *form)
{
	const char *p = strfrmt;

	while (strchr(FLAGS, *p)) {
		p++;							/* skip flags */
	}

	if ((size_t)(p - strfrmt) >= sizeof(FLAGS)) {
		luaL_error(L, "invalid format (repeated flags)");
	}

	if (isdigit(uchar(*p))) {
		p++;							/* skip width */
	}

	if (isdigit(uchar(*p))) {
		p++;							/* (2 digits at most) */
	}

	if (*p == '.') {
		p++;

		if (isdigit(uchar(*p))) {
			p++;								/* skip precision */
		}

		if (isdigit(uchar(*p))) {
			p++;								/* (2 digits at most) */
		}
	}

	if (isdigit(uchar(*p))) {
		luaL_error(L, "invalid format (width or precision too long)");
	}

	*(form++) = '%';
	strncpy(form, strfrmt, p - strfrmt + 1);
	form += p - strfrmt + 1;
	*form = '\0';
	return p;
}

static inline
void _addintlen(char *form)
{
	size_t  l = strlen(form);
	char    spec = form[l - 1];

	strcpy(form + l - 1, LUA_INTFRMLEN);
	form[l + sizeof(LUA_INTFRMLEN) - 2] = spec;
	form[l + sizeof(LUA_INTFRMLEN) - 1] = '\0';
}

/**
 * 将L栈中的字符串格式和参数解析。
 * 如果失败则返回0，栈保存不变
 * 如果成功则返回1，并将结果整合到栈顶的luaL_Buffer中。
 */
int luaL_stringFormat(lua_State *L)
{
	int     arg = 1;
	size_t  sfl;

	if (lua_isnil(L, arg)) {
		return 0;
	}

	const char      *strfrmt = luaL_checklstring(L, arg, &sfl);
	const char      *strfrmt_end = strfrmt + sfl;
	luaL_Buffer     b;

	luaL_buffinit(L, &b);

	while (strfrmt < strfrmt_end) {
		if (*strfrmt != L_ESC) {
			luaL_addchar(&b, *strfrmt++);
		} else if (*++strfrmt == L_ESC) {
			luaL_addchar(&b, *strfrmt++);				/* %% */
		} else {							/* format item */
			char    form[MAX_FORMAT];				/* to store the format (`%...') */
			char    buff[MAX_ITEM];					/* to store the formatted item */
			arg++;
			strfrmt = _scanformat(L, strfrmt, form);
			switch (*strfrmt++)
			{
				case 'c':
				{
					if (lua_isnil(L, arg)) {
						snprintf(buff, sizeof(buff), "nil");
					} else {
						snprintf(buff, sizeof(buff), form, (int)luaL_checknumber(L, arg));
					}

					break;
				}

				case 'd':
				case 'i':
				{
					if (lua_isnil(L, arg)) {
						snprintf(buff, sizeof(buff), "nil");
					} else {
						_addintlen(form);
						snprintf(buff, sizeof(buff), form, (LUA_INTFRM_T)luaL_checknumber(L, arg));
					}

					break;
				}

				case 'o':
				case 'u':
				case 'x':
				case 'X':
				{
					if (lua_isnil(L, arg)) {
						snprintf(buff, sizeof(buff), "nil");
					} else {
						_addintlen(form);
						snprintf(buff, sizeof(buff), form, (unsigned LUA_INTFRM_T)luaL_checknumber(L, arg));
					}

					break;
				}

				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
				{
					if (lua_isnil(L, arg)) {
						snprintf(buff, sizeof(buff), "nil");
					} else {
						snprintf(buff, sizeof(buff), form, (double)luaL_checknumber(L, arg));
					}

					break;
				}

				case 'q':
				{
					_addquoted(L, &b, arg);
					continue;						/* skip the 'addsize' at the end */
				}

				case 's':
				{
					if (lua_isnil(L, arg)) {
						snprintf(buff, sizeof(buff), "nil");
					} else {
						size_t          l;
						const char      *s = luaL_checklstring(L, arg, &l);

						if (!strchr(form, '.') && (l >= 100)) {
							/* no precision and string is too long to be formatted;
							 *   keep original string */
							lua_pushvalue(L, arg);
							luaL_addvalue(&b);
							continue;								/* skip the `addsize' at the end */
						} else {
							snprintf(buff, sizeof(buff), form, s);
						}
					}

					break;
				}

				default:
				{							/* also treat cases `pnLlh' */
					return luaL_error(L, "invalid option to " LUA_QL("format"));
				}
			}
			luaL_addlstring(&b, buff, strlen(buff));
		}
	}

	luaL_pushresult(&b);
	return 1;
}

/* -----------          */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source. */
void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
{
	int i;

	luaL_checkstack(l, nup, "too many upvalues");

	for (; reg->name != NULL; reg++) {			/* fill the table with given functions */
		for (i = 0; i < nup; i++) {			/* copy upvalues to the top */
			lua_pushvalue(l, -nup);
		}

		lua_pushcclosure(l, reg->func, nup);			/* closure with those upvalues */
		lua_setfield(l, -(nup + 2), reg->name);
	}

	lua_pop(l, nup);	/* remove upvalues */
}
#endif	/* if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502 */

void luaL_tableAddTable(lua_State *L, const char *name, const char *metaname, bool isweak)
{
	assert(L);

	if (!lua_istable(L, -1)) {
		luaL_error(L, "expected `table` argument.");
	}

	lua_newtable(L);

	if (metaname && (metaname[0] != '\0')) {
		luaL_newmetatable(L, metaname);

		if (isweak) {
			lua_pushstring(L, "v");
			lua_setfield(L, -2, "__mode");
		}

		lua_pushstring(L, "Not your business.");
		lua_setfield(L, -2, "__metatable");

		lua_setmetatable(L, -2);
	}

	if (name && (name[0] != '\0')) {
		lua_setfield(L, -2, name);
	} else {
		size_t tsize = lua_objlen(L, -2);
		lua_pushinteger(L, tsize + 1);
		lua_insert(L, -2);
		lua_settable(L, -2);
	}
}

char *luaL_getProgName(lua_State *L, char *buff, size_t size)
{
	/*禁止添加日志相关文件*/
	void *stackptr[1] = {};

	assert(buff);
	assert(size);

	lua_getglobal(L, "getprogname");

	if (lua_iscfunction(L, -1)) {
		if (lua_pcall(L, 0, 1, 0) == 0) {
			const char *ptr = luaL_checkstring(L, -1);
			snprintf(buff, size, "%s", ptr);

			lua_pop(L, 2);

			return buff;
		} else {
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);

	backtrace(stackptr, DIM(stackptr));
#ifdef _GNU_SOURCE
	Dl_info info = {};

	if (dladdr(stackptr[0], &info)) {
		const char      *ptr1 = NULL;
		const char      *ptr2 = NULL;
		ptr1 = info.dli_fname ? info.dli_fname : "NULL";
		ptr2 = strrchr(ptr1, '/');
		ptr2 = ptr2 ? (ptr2 + 1) : ptr1;
		snprintf(buff, size, "%s", ptr2);
	} else {
		snprintf(buff, size, "unkown");
	}

#else
	char **info = NULL;

	info = backtrace_symbols(stackptr, DIM(stackptr));

	if (info) {
		const char      *ptr = NULL;
		const char      *next = NULL;
  #ifdef __LINUX__
		bzero(buff, size);

		ptr = strrchr(info[0], '/');

		if (likely(ptr)) {
			next = strrchr(++ptr, '(');

			if (likely(next)) {
				snprintf(buff, size, "%.*s", (int)(next - ptr), ptr);
			}
		}

		if (unlikely(buff[0] == '\0')) {
			snprintf(buff, size, "unkown");
		}

  #else
		ptr = strchr(info[0], ' ');
		ptr = ptr ? (ptr + 1) : info[0];

		while (*ptr == ' ') {
			ptr++;
		}

		next = strchr(ptr, ' ');

		if (next) {
			snprintf(buff, size, "%.*s", (int)(next - ptr), ptr);
		} else {
			snprintf(buff, size, "%s", ptr);
		}
  #endif		/* ifdef __LINUX__ */
		free(info);
	} else {
		snprintf(buff, size, "unkown");
	}
#endif	/* ifdef _GNU_SOURCE */

	return buff;
}

