#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>

typedef void (EVENTS_FUNCTION)(void *argv);

#define EVENT_MAP(XX)		       \
	XX(EVCS_EVENT_ROOT, "起源事件")    \
	XX(EVCS_EVENT_INIT, "初始化事件")   \
	XX(EVCS_EVENT_WORK, "任务环事件")   \
	XX(EVCS_EVENT_WARINIG, "警告信息") \
	XX(EVCS_EVENT_OTHER, "其它信息")

typedef enum __event_type_t
{
#define XX(name, _) name,
	EVENT_MAP(XX)
#undef XX
	W_EVENT_EOF,
} event_type_t;

struct evcs_events
{
	EVENTS_FUNCTION *evcb[W_EVENT_EOF];
};
#define evcs_events_t __thread struct evcs_events

#define EVENT_NAME_GEN(name, _)	\
	case name:		\
		return #name;

static inline
const char *event_getname(int type)
{
	switch (type)
	{
		EVENT_MAP(EVENT_NAME_GEN)
		default:
			return NULL;
	}
}

#undef EVENT_NAME_GEN

#define EVENT_INFO_GEN(name, info) \
	case name:		   \
		return info;

static inline
const char *event_getinfo(int type)
{
	switch (type)
	{
		EVENT_MAP(EVENT_INFO_GEN)
		default:
			return "未知信息";
	}
}

#undef EVENT_INFO_GEN

#define EVENT_PRINT_GEN(name, info) printf("value:%d, name:%s, info:%s\n", name,#name, info);
static inline
void event_print(void)
{
	EVENT_MAP(EVENT_PRINT_GEN)
}

#undef EVENT_PRINT_GEN

static inline
void event_work(struct evcs_events *evts, int type, void *data)
{
	if (evts && evts->evcb[type]) {
		printf("\t\t\tEVENT[%s]\t\t\thappend!\n", event_getinfo(type));
		evts->evcb[type](data);
	}
}

#ifdef __cplusplus
}
#endif

