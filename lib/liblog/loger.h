#ifndef _LOGER_H
#define _LOGER_H

#include "./slog/slog_api.h"

#include "string.h"

#define WATCH_DELAY_TIME 10 * 1000
// #define MODULE_NAME "CoreExchangeNode"

extern struct CSLog *g_imlog;

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#define loger(fmt, args ...)    Info(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define trace(fmt, args ...)    Trace(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define debug(fmt, args ...)    Debug(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define warn(fmt, args ...)     Warn(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define error(fmt, args ...)    Error(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define fatal(fmt, args ...)    Fatal(g_imlog, "<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#endif

