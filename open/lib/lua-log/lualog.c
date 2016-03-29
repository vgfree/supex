//
//  lualog.c
//  bLib
//
//  Created by 周凯 on 15/8/8.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#include "lualog_utils.h"

// #define ADD_LENGTH_FIELD

#ifdef ADD_LENGTH_FIELD
  #define LENGTH_FIELD_FMTSIZE  (6)
#else
  #define LENGTH_FIELD_FMTSIZE  (0)
#endif

/* -----------          */
#define SPLITLOG_BY_DAY
/* -----------          */

#define MAX_PATH_SIZE           (512)
#define MAX_NAME_SIZE           (32)

#define DEFAULT_LOGPATH_VALUE   (".")
#define DEFAULT_LOG_KEY         (".")
#define START_STLAYER           (1)
/* -----------          */

/*
 * 字段和其对应的元表、C结构
 */

/*日志文件路径*/
#define EXPORT_LOGPATH_KEY      "logpath"
#define EXPORT_LOGPATH_MT       "LUALOG.LOGPATH.MT"
/*日志级别*/
#define EXPORT_LEVEL_KEY        "level"
#define EXPORT_LEVEL_MT         "LUALOG.LEVEL.MT"

struct LogLevel
{
	int             level;
	const char      *shortname;
	const char      *name;
};

static const struct LogLevel gLevel[] = {
	{ 1, "D", "DEBUG" },
	{ 2, "I", "INFO " },
	{ 3, "W", "WARN " },
	{ 4, "E", "ERROR" },
	{ 5, "S", "SYS  " },
	{ 6, "F", "FATAL" },
};

#define LOGLEVEL_SHORTNAME_LIST "['D', 'I', 'W', 'E', 'S',' F']"

#define LOGLEVEL_SHORTNAME_ERR  "log level must be "LOGLEVEL_SHORTNAME_LIST

/*导出表元表*/
#define EXPORT_LIB_MT           "LUALOG.MT"
/*日志句柄池*/
#define EXPORT_POOL_KEY         "pool"
#define EXPORT_POOL_MT          "LUALOG.POOL.MT"

/*日志句柄*/
#define EXPORT_LOGHDL_KEY       "loghdl"
#define EXPORT_LOGHDL_MT        "LUALOG.LOGHDL.MT"

struct LogHdl
{
	int             fd;
	int             totalsize;
	int             cursize;
	struct tm       creattm;
	char            name[MAX_NAME_SIZE];
	char            path[MAX_PATH_SIZE];
};

/*附加信息字段*/
#define EXPORT_ADDINFO_KEY      "additional"
#define EXPORT_ADDINFO_MT       "LUALOG.ADDITIONAL.MT"

static inline
long _gettid()
{
	long tid = -1;

#if defined(__APPLE__)
	tid = syscall(SYS_thread_selfid);
#else
	tid = syscall(SYS_gettid);
#endif
	return tid;
}

/* -----------          */

/**
 * 初始化结构
 */
static int _initlog(struct LogHdl *loghdl, const char *defaultpath, const char *logpath);

/**
 * 参数为日志文件名称,如果不包含路径则使用默认路径
 */
static int _openlog(lua_State *L);

/**
 * 无参数
 */
static int _closelog(lua_State *L);

/**
 * 当有参数时,作为句柄名称,并关闭对应的日志句柄
 */
static int _closeall(lua_State *L);

/**
 * 无参数
 */
static int _flushlog(lua_State *L);

/**
 * 写日志
 */
static int _writelogtofile(lua_State *L, struct LogHdl *loghdl,
	const char *levelname,
	const char *msg, size_t size,
	struct timeval *tv,
	int stlayer);

/**
 * 参数为(日志句柄、日志等级、日志信息)，起始栈层
 */
static int _writelogbyhdlwrap(lua_State *L, int stlayer);

/**
 * 参数为(日志句柄、日志等级、日志信息)
 */
static int _writelogbyhdl(lua_State *L);

/**
 * 参数为(日志等级、日志信息)
 */
static int _writelog(lua_State *L);

/**
 * 参数为(日志句柄、日志等级、日志信息)
 */
static int _writefulllogbyhdl(lua_State *L);

/**
 * 参数为(日志等级、日志信息)
 */
static int _writefulllog(lua_State *L);

/**
 * 添加附加信息
 */
static int _addinfo(lua_State *L);

/**
 * 设置输出级别
 */
static int _setlevel(lua_State *L);

/**
 * 设置日志路径
 */
static int _setpath(lua_State *L);

/*
 * 检查栈底的日志级别参数
 */
static const struct LogLevel    *_checklevel(lua_State *L);

/* ------------------                           */

/**
 * @param defaultpath 为默认路径,在调用则只提供了文件名时使用
 * @param path 为null时重新使用句柄内部信息打开日志句柄，否则用新路径打开句柄
 */
static int _initlog(struct LogHdl *loghdl, const char *defaultpath, const char *path)
{
	struct timeval  tv = {};
	struct tm       tm = {};
	char            fullname[MAX_PATH_SIZE] = {};
	char            strtm[32] = {};
	int             fd = -1;

	assert(loghdl);

	if (path && (path[0] != '\0')) {
		const char *basename = NULL;

		basename = strrchr(path, '/');

		if (basename) {
			basename += 1;
			snprintf(loghdl->path, sizeof(loghdl->path), "%.*s", (int)(basename - path - 1), path);
			snprintf(loghdl->name, sizeof(loghdl->name), "%s", basename);
		} else {
			defaultpath = defaultpath ? defaultpath : DEFAULT_LOGPATH_VALUE;
			snprintf(loghdl->path, sizeof(loghdl->path), "%s", defaultpath);
			snprintf(loghdl->name, sizeof(loghdl->name), "%s", path);
		}
	} else if (loghdl->name[0] == '\0') {
		errno = EINVAL;
		return -1;
	}

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);

#ifdef SPLITLOG_BY_DAY
	strftime(strtm, sizeof(strtm), "%Y%m%d", &tm);
#else
	strftime(strtm, sizeof(strtm), "%Y%m", &tm);
#endif

	memcpy(&loghdl->creattm, &tm, sizeof(tm));
#if 0
	if (strftime(strtm, sizeof(strtm), "%Y%m", &tm) == 0) {
		snprintf(strtm, sizeof(strtm), "%s", "197701");
		memset(&loghdl->creattm, 0, sizeof(loghdl->creattm));
		loghdl->creattm.tm_year = 1977;
		loghdl->creattm.tm_mon = 0;
		loghdl->creattm.tm_mday = 1;
	} else {
		memcpy(&loghdl->creattm, &tm, sizeof(tm));
	}
#endif
	snprintf(fullname, sizeof(fullname), "%s/%s_%s.log", loghdl->path, strtm, loghdl->name);

	fd = open(fullname, O_CREAT | O_APPEND | O_WRONLY, 0644);

	if (fd < 0) {
		return -1;
	}

	if (loghdl->fd > 0) {
		close(loghdl->fd);
	}

	loghdl->fd = fd;

	return fd;
}

/*
 * 检查栈底的日志级别参数
 */
static const struct LogLevel *_checklevel(lua_State *L)
{
	int                     i = 0;
	const char              *str = NULL;
	size_t                  strl = 0;
	const struct LogLevel   *level = NULL;

	str = luaL_checklstring(L, 1, &strl);

	luaL_argcheck(L, strl == 1, 2, LOGLEVEL_SHORTNAME_ERR);

	for (i = 0; i < DIM(gLevel); i++) {
		if (gLevel[i].shortname[0] == str[0]) {
			break;
		}
	}

	luaL_argcheck(L, i < DIM(gLevel), 2, LOGLEVEL_SHORTNAME_ERR);

	/*获取当前的日志级别*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_LEVEL_KEY);
	level = (const struct LogLevel *)lua_touserdata(L, -1);
	/*弹出不需要的值*/
	lua_pop(L, 1);

	if (!level) {
		luaL_error(L, "This a bug, Please report.");
	}

	if (level->level > gLevel[i].level) {
		level = NULL;
	} else {
		level = &gLevel[i];
	}

	return level;
}

/**
 * @param L 该函数只使用其中的栈调用路径
 * @param loghdl 日志输出句柄，如果为NULL则输出到终端
 * @param levelname 日志级别名称
 * @param msg 输出的消息
 * @param tv 输出的时间戳
 * @param stlayer 需要获取的栈层，如果小于0则获取所有的调用栈路径
 */
static int _writelogtofile(lua_State *L, struct LogHdl *loghdl,
	const char *levelname,
	const char *msg,
	size_t size,
	struct timeval *tv,
	int stlayer)
{
	lua_Debug       debug = {};
	luaL_Buffer     buffer = {};
	char            funcinfo[128] = { 0 };
	char            strtm[32] = { 0 };
	char            progName[32] = { 0 };
	const char      *src = NULL;
	const char      *addinfo = NULL;
	const char      *defpath = NULL;
	struct tm       tm = {};
	int             fd = -1;
	int             i = 0;
	size_t          nbytes = 0;
	size_t          ailen = 0;

	assert(L);
	assert(tv);
	assert(levelname);

	/*是否注册过获取程序名函数*/
	lua_getglobal(L, "GetProgName");

	/*获取默认路径*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_LOGPATH_KEY);		// -2
	defpath = luaL_checkstring(L, -1);

	/*整合附件字段信息*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_ADDINFO_KEY);		// -1
	luaL_buffinit(L, &buffer);
	/*遍历表*/
	lua_pushnil(L);

	while (lua_next(L, -2) != 0) {
		luaL_addstring(&buffer, luaL_checkstring(L, -1));
		luaL_addchar(&buffer, ',');
		/*弹出value*/
		lua_pop(L, 1);
	}

	luaL_pushresult(&buffer);	// -1
	addinfo = luaL_checklstring(L, -1, &ailen);

	/*转换时间*/
	localtime_r(&tv->tv_sec, &tm);

	if ((nbytes = strftime(strtm, sizeof(strtm), "%Y-%m-%d %H:%M:%S", &tm)) == 0) {
		snprintf(strtm, sizeof(strtm), "1977-01-01 00:00:00.000000");
	} else {
		snprintf(strtm + nbytes, sizeof(strtm) - nbytes, ".%06ld", (long)tv->tv_usec);
	}

	/*是否需要创建新文件*/
	if (loghdl) {
#ifdef SPLITLOG_BY_DAY
		if ((tm.tm_year != loghdl->creattm.tm_year * 12) ||
			(tm.tm_yday != loghdl->creattm.tm_yday)) {
#else
		int curmon = tm.tm_year * 12 + tm.tm_mon;
		int crtmon = loghdl->creattm.tm_year * 12 + loghdl->creattm.tm_mon;

		if (curmon != crtmon) {
#endif
			fd = _initlog(loghdl, defpath, NULL);
		} else {
			fd = loghdl->fd;
		}
	}

	/*如果没有指定文件句柄，或者刷新失败，则记录到终端*/
	if (fd < 0) {
		fd = STDOUT_FILENO;
	}

	i = stlayer > 0 ? stlayer : START_STLAYER;

	while (1) {
		/*
		 * 获取调用栈信息
		 */
		if (!(lua_getstack(L, i, &debug) && lua_getinfo(L, "Sln", &debug))) {
			return -1;
		}

		if (debug.what && (strcasecmp(debug.what, "main") == 0)) {
			snprintf(funcinfo, sizeof(funcinfo), "main()");
		} else if (debug.what && (strcasecmp(debug.what, "lua") == 0)) {
			// snprintf(funcinfo, sizeof(funcinfo), "Lua : %s, %s()",
			//          debug.namewhat && debug.namewhat[0] != '\0' ? debug.namewhat : "@undefine",
			//          debug.name && debug.name[0] != '\0' ? debug.name : "@unkown");

			snprintf(funcinfo, sizeof(funcinfo), "%s()",
				debug.name && debug.name[0] != '\0' ? debug.name : "@unkown");
		} else if (debug.what && (strcasecmp(debug.what, "c") == 0)) {
			snprintf(funcinfo, sizeof(funcinfo), "C : %s()",
				debug.name && debug.name[0] != '\0' ? debug.name : "@unkown");
		} else {
			snprintf(funcinfo, sizeof(funcinfo), "@v@ BUG,Please report this to me! @v@");
			stlayer = stlayer < 0 ? START_STLAYER : stlayer;
		}

		/*
		 * 记录日志
		 * 固定字段：栈层、时间戳、函数名、文件名、行数、日志级别、
		 * 附件字段：附加信息、用户信息
		 */
		src = strrchr(debug.short_src, '/');
		src = src ? (src + 1) : debug.short_src;
#ifdef ADD_LENGTH_FIELD
		int bytes = 0;
		bytes = snprintf(NULL, 0, "|%s|%s|%16s.%06ld.%03d|%24s|%32s:%4d|(%.*s) : %s\n",
				levelname,
				strtm,
				luaL_getProgName(L, progName, sizeof(progName)),
				_gettid(), i,
				funcinfo,
				src,
				debug.currentline,
				(int)(ailen > 0 ? ailen - 1 : ailen),
				addinfo,
				msg ? msg : "***");

		if (bytes > pow(10, LENGTH_FIELD_FMTSIZE) - 1) {
			dprintf(STDERR_FILENO, "%24s|%32s:%4d|length of message about log is too large.",
				funcinfo,
				src,
				debug.currentline);
			break;
		} else {
#else
		{
#endif			/* ifdef ADD_LENGTH_FIELD */
			dprintf(fd,
#ifdef ADD_LENGTH_FIELD
				"%-*d|%s|%s|%16s.%06ld.%03d|%24s|%32s:%4d|(%.*s) : %s\n",
				LENGTH_FIELD_FMTSIZE,
				bytes,
#else
				"|%s|%s|%16s.%06ld.%03d|%24s|%32s:%4d|(%.*s) : %s\n",
#endif
				levelname,
				strtm,
				luaL_getProgName(L, progName, sizeof(progName)),
				_gettid(), i,
				funcinfo,
				src,
				debug.currentline,
				(int)(ailen > 0 ? ailen - 1 : ailen),
				addinfo,
				msg ? msg : "***");

			if (stlayer > -1) {
				break;
			}

			i++;

			msg = NULL;
		}
	}

	lua_pop(L, 3);

	return 0;
}

static
const luaL_Reg logReg[] = {
	//        { "close",     _closelog               },
	{ "flush",     _flushlog               },
	{ "write",     _writelogbyhdl          },
	{ "writefull", _writefulllogbyhdl      },
	{ NULL,        NULL                    }
};

static int _closelog(lua_State *L)
{
	struct LogHdl   *loghdl = NULL;
	struct LogHdl   *ptr = NULL;

	loghdl = (struct LogHdl *)luaL_checkudata(L, 1, EXPORT_LOGHDL_MT);

#ifdef DEBUG
	char fullname[MAX_PATH_SIZE] = {};
	snprintf(fullname, sizeof(fullname), "%s/%s", loghdl->path, loghdl->name);
	dprintf(STDOUT_FILENO, "release the handle of log : %s.\n", fullname);
#endif

	/*获取句柄池*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_POOL_KEY);		// -2
	/*当前默认句柄*/
	lua_getfield(L, -1, DEFAULT_LOG_KEY);				// -1

	if (lua_type(L, -1) != LUA_TNIL) {
		ptr = (struct LogHdl *)luaL_checkudata(L, -1, EXPORT_LOGHDL_MT);

		if (ptr == loghdl) {
			lua_pushnil(L);
			lua_setfield(L, -3, DEFAULT_LOG_KEY);
		}
	}

	/*抹去句柄池中对应的句柄*/
	lua_pushnil(L);
	lua_setfield(L, -3, loghdl->name);

	if (loghdl->fd > -1) {
		close(loghdl->fd);
		loghdl->fd = -1;
	}

	return 0;
}

static int _flushlog(lua_State *L)
{
	struct LogHdl *loghdl = NULL;

	loghdl = (struct LogHdl *)luaL_checkudata(L, 1, EXPORT_LOGHDL_MT);

	/*初始化*/
	if (_initlog(loghdl, NULL, NULL) < 0) {
		return luaL_error(L, "can't flush log : %s.", strerror(errno));
	}

	return 0;
}

static int _openlog(lua_State *L)
{
	int             argc = 0;
	const char      *file = NULL;
	const char      *defpath = NULL;
	struct LogHdl   *loghdl = NULL;

	argc = lua_gettop(L);

	if (argc != 1) {
		return luaL_error(L, "expected 1 arguments.");
	}

	file = luaL_checkstring(L, 1);

	if ((file[0] == '.') && (file[1] == '\0')) {
		return luaL_error(L, "file name [.] invaild.");
	}

	/*获取默认路径*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_LOGPATH_KEY);		// -3
	defpath = luaL_checkstring(L, -1);

	/*查找是否存储该路径对应的句柄*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_POOL_KEY);		// -2
	lua_getfield(L, -1, file);					// -1

	loghdl = lua_touserdata(L, -1);					// -1

	if (loghdl == NULL) {
		/*没有该日志对应的句柄,则新建一个*/

		/*弹出无效的值*/
		lua_pop(L, 1);

		/*创建lua的自动对象，并设置元方法*/
		loghdl = (struct LogHdl *)lua_newuserdata(L, sizeof(*loghdl));			// -1

		loghdl->fd = -1;
		/*设置对象的元方法*/
		{
			luaL_newmetatable(L, EXPORT_LOGHDL_MT);

			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");

			lua_pushvalue(L, lua_upvalueindex(1));
			lua_pushcclosure(L, _closelog, 1);
			lua_setfield(L, -2, "__gc");

			lua_pushstring(L, "Not your business.");
			lua_setfield(L, -2, "__metatable");

			lua_pushvalue(L, lua_upvalueindex(1));
			luaL_setfuncs(L, logReg, 1);

			lua_setmetatable(L, -2);
		}

		/*初始化自动对象*/
		if (_initlog(loghdl, defpath, file) < 0) {
			return luaL_error(L, "can't open log : %s.", strerror(errno));
		}

#ifdef DEBUG
		dprintf(STDOUT_FILENO, "open the handle of log %s.\n", loghdl->name);
#endif
		/*设置到句柄池*/
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, loghdl->name);
	} else {
		/*有该日志对应的句柄,则检查是否是正确的句柄*/
		loghdl = luaL_checkudata(L, -1, EXPORT_LOGHDL_MT);
	}

	/*设置为默认句柄*/
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, DEFAULT_LOG_KEY);

	return 1;
}

/*
 * ARG: loghdl, level, fmt, ...
 */
static int _writelogbyhdlwrap(lua_State *L, int stlayer)
{
	struct LogHdl           *loghdl = NULL;
	const struct LogLevel   *level = NULL;
	struct timeval          tv = {};
	const char              *str = NULL;
	size_t                  strlen = 0;

	/*在没有打开一个日志句柄的时候,获取的默认句柄为NIL,则输出到终端*/
	if (lua_type(L, 1) != LUA_TNIL) {
		loghdl = (struct LogHdl *)luaL_checkudata(L, 1, EXPORT_LOGHDL_MT);
	} else {
		loghdl = NULL;
	}

	/*移除栈底的1个参数,已匹配_checklevel函数的参数格式*/
	lua_remove(L, 1);

	level = _checklevel(L);

	if (level == NULL) {
		return 0;
	}

	/*移除栈底的1个参数,已匹配luaL_stringFormat函数的参数格式*/
	lua_remove(L, 1);

	if (luaL_stringFormat(L) == 0) {
		str = "NIL";
		strlen = sizeof(str) - 1;
	} else {
		str = luaL_checklstring(L, -1, &strlen);
	}

	gettimeofday(&tv, NULL);

	_writelogtofile(L, loghdl, level->name, str, strlen, &tv, stlayer);

	return 0;
}

/* ---------------              */

/*
 * lua调用格式 : loghdl, level, fmt, ...
 */
static int _writelogbyhdl(lua_State *L)
{
	int argc = -1;

	argc = lua_gettop(L);

	if (argc < 3) {
		return luaL_error(L, "expected >=3 arguments.");
	}

	return _writelogbyhdlwrap(L, START_STLAYER);
}

/*
 * lua调用格式 : loghdl, level, fmt, ...
 */
static int _writefulllogbyhdl(lua_State *L)
{
	int argc = -1;

	argc = lua_gettop(L);

	if (argc < 3) {
		return luaL_error(L, "expected >=3 arguments.");
	}

	return _writelogbyhdlwrap(L, -1);
}

/* ---------------              */

/*
 * lua调用格式 : level, fmt, ...
 */
static int _writelog(lua_State *L)
{
	int argc = -1;

	argc = lua_gettop(L);

	if (argc < 2) {
		return luaL_error(L, "expected >=2 arguments.");
	}

	/*获取默认句柄，在没有打开一个日志句柄的时候，获取的默认句柄为NIL*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_POOL_KEY);
	lua_getfield(L, -1, DEFAULT_LOG_KEY);
	lua_insert(L, 1);
	lua_pop(L, 1);

	_writelogbyhdlwrap(L, START_STLAYER);

	return 0;
}

/*
 * lua调用格式 : level, fmt, ...
 */
static int _writefulllog(lua_State *L)
{
	int argc = -1;

	argc = lua_gettop(L);

	if (argc < 2) {
		return luaL_error(L, "expected >=2 arguments.");
	}

	/*获取默认句柄，在没有打开一个日志句柄的时候，获取的默认句柄为NIL*/
	lua_getfield(L, lua_upvalueindex(1), EXPORT_POOL_KEY);
	lua_getfield(L, -1, DEFAULT_LOG_KEY);
	lua_insert(L, 1);
	lua_pop(L, 1);

	_writelogbyhdlwrap(L, -1);

	return 0;
}

/**
 * 参数(名称、值(可以是lua变量))
 * 当没有参数时清零附件信息
 */
static int _addinfo(lua_State *L)
{
	int     argc = 0;
	int     i = 0;
	size_t  tsize = 0;

	argc = lua_gettop(L);

	if (argc < 1) {
		/*清除值*/
		// return 0;
	}

	lua_getfield(L, lua_upvalueindex(1), EXPORT_ADDINFO_KEY);			// -2

	tsize = lua_objlen(L, -1);

	/*覆盖旧数据*/
	for (i = 1; i < (argc + 1); i++) {
		lua_pushvalue(L, i);			// -1
		lua_rawseti(L, -2, i);
	}

	/*清除旧数据*/
	for (; i < (tsize + 1); i++) {
		lua_pushnil(L);
		lua_rawseti(L, -2, i);
	}

	return 0;
}

static int _closeall(lua_State *L)
{
	int             argc = lua_gettop(L);
	const char      *name = NULL;

	if (argc < 1) {
		/* close all */

		/*
		 * 创建一个日志句柄池
		 * "."对应的句柄为当前默认日志输出句柄
		 */
		lua_pushvalue(L, lua_upvalueindex(1));
		luaL_tableAddTable(L, EXPORT_POOL_KEY, EXPORT_POOL_MT, false);
	} else {
		if (argc != 1) {
			return luaL_error(L, "expected 1 arguments.");
		}

		name = luaL_checkstring(L, 1);

		/* close by name of log handle*/
		lua_getfield(L, lua_upvalueindex(1), EXPORT_POOL_KEY);
		lua_pushnil(L);
		lua_setfield(L, -2, name);
	}

	return 0;
}

static int _setlevel(lua_State *L)
{
	int     argc = -1;
	int     level = 0;

	argc = lua_gettop(L);

	if (argc != 1) {
		return luaL_error(L, "expected 1 arguments.");
	}

	level = luaL_checkint(L, 1);

	level = level < 0 ? 0 : (level > (DIM(gLevel) - 1) ? (DIM(gLevel) - 1) : level);

	lua_pushlightuserdata(L, (void *)&gLevel[level]);
	lua_setfield(L, lua_upvalueindex(1), EXPORT_LEVEL_KEY);

	return 0;
}

static int _setpath(lua_State *L)
{
	int argc = -1;

	size_t          strl = 0;
	const char      *ptr = NULL;

	argc = lua_gettop(L);

	if (argc != 1) {
		return luaL_error(L, "expected 1 arguments.");
	}

	ptr = luaL_checklstring(L, 1, &strl);

	luaL_argcheck(L, strl > 0, 1, "log path must not be null string.");

	if ((mkdir(ptr, 0766) < 0) && (errno != EEXIST)) {
		luaL_error(L, "log path %s is invalid : %s", ptr, strerror(errno));
	}

	lua_setfield(L, lua_upvalueindex(1), EXPORT_LOGPATH_KEY);

	return 0;
}

/* -----------          */

static
const luaL_Reg libReg[] = {
	{ "open",      _openlog         },
	{ "write",     _writelog        },
	{ "writefull", _writefulllog    },
	{ "addinfo",   _addinfo         },
	{ "close",     _closeall        },
	{ "setlevel",  _setlevel        },
	{ "setpath",   _setpath         },
	{ NULL,        NULL             }
};

int luaopen_lualog(lua_State *L)
{
	/* ---------------      */

	/*创建导出表*/

	lua_newtable(L);

	/*
	 * 将导出表自身设置成为每个导出函数的upvalue
	 */
	lua_pushvalue(L, -1);
	luaL_setfuncs(L, libReg, 1);

	/*
	 * 创建一个日志句柄池
	 * "."对应的句柄为当前默认日志输出句柄
	 */
	luaL_tableAddTable(L, EXPORT_POOL_KEY, EXPORT_POOL_MT, false);

	luaL_newmetatable(L, EXPORT_LIB_MT);
	/* ---------------      */
	/* 私有数据,不可以在lua层更改*/

	/*日志追加输出字段*/
	luaL_tableAddTable(L, EXPORT_ADDINFO_KEY, EXPORT_ADDINFO_MT, true);

	/*
	 * 设置一个打印级别
	 * 一共5个级别，小于该设置级别的日志将不会打印
	 */
	lua_pushlightuserdata(L, (void *)&gLevel[0]);
	lua_setfield(L, -2, EXPORT_LEVEL_KEY);

	/*
	 * 日志文件路径
	 */
	lua_pushstring(L, DEFAULT_LOGPATH_VALUE);
	lua_setfield(L, -2, EXPORT_LOGPATH_KEY);

	/* ---------------      */

	/*
	 * 对lua层隐藏私有数据
	 */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushstring(L, "Not your business");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);

	/* ---------------      */
	return 1;
}

