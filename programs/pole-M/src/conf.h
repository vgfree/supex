#pragma once

/* Log level */
#define LOG_DEBUG       0
#define LOG_INFO        1
#define LOG_WARN        2
#define LOG_ERROR       3
#define LOG_SYS         4
#define LOG_FATAL       5

typedef struct
{
	char    input_uri[32];
	char    bind_uri[32];
	char    log_path[256];
	int     log_level;
	int     max_records;
	int     thread_number; // 如果宏编译TC_COROUTINE 则thread_number生效;
} config_t;

extern config_t g_pole_conf;

int config_init(const char *path);

