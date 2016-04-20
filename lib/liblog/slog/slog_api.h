#ifndef __slog__slog_api__
#define __slog__slog_api__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct CSLogObject;
    struct CSLog {
      struct CSLogObject* m_log;
    };
    struct CSLog* CSLog_create(const char* module_name, int delay);
    void CSLog_destroy(struct CSLog* log);
    
    void Trace(struct CSLog* log, const char* format, ...);
    void Debug(struct CSLog* log, const char* format, ...);
    void Info(struct CSLog* log, const char* format, ...);
    void Warn(struct CSLog* log, const char* format, ...);
    void Error(struct CSLog* log, const char* format, ...);
    void Fatal(struct CSLog* log, const char* format, ...);
 
#ifdef __cplusplus
};
#endif


#endif /* defined(__slog__slog_api__) */
