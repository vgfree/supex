#pragma once

#define _FL_            __FILE__, __func__, __LINE__
#define LOG_D           0, _FL_	/*调试*/
#define LOG_I           1, _FL_	/*提示*/
#define LOG_W           2, _FL_	/*警告*/
#define LOG_F           3, _FL_	/*错误*/
#define LOG_E           4, _FL_	/*严重错误*/
#define LOG_S           5, _FL_	/*系统提示信息*/
#define LOG_SE          6, _FL_	/*系统错误*/

#define LOG_DEBUG       "DEBUG"
#define LOG_INFO        "INFO"
#define LOG_WARN        "WARN"
#define LOG_FAIL        "FAIL"
#define LOG_ERR         "ERROR"
#define LOG_SYSI        "SYSINFO"
#define LOG_SYSE        "SYSERROR"

extern int      g_log_level;	/*打印日志的级别*/
extern unsigned g_log_max;	/*允许创建的日志文件数量*/

/*在每个线程启动前为其初始化一个日志文件，若果没有如初始化，日志输出到标准输出*/
int log_init(char *file);

/*
 *   典型调用：log_info(LOG_I,"hello world[%d]\n",5);
 */
void log_info(int level, const char *file, const char *func, int line, char *fmt, ...);

