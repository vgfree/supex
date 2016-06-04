#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <unistd.h>
#include "evcs_events.h"

typedef void (MODULE_FUNCTION)(void *self);

struct evcs_module
{
	char                    *name;
	bool                    state;
	MODULE_FUNCTION         *init;
	MODULE_FUNCTION         *exit;
	struct evcs_module      *next;
	struct evcs_events      *evts;
	void                    *argv;
};
#define evcs_module_t __thread struct evcs_module

static __thread struct evcs_module *g_module_root = { 0 };

#define EVCS_MODULE_SETUP(name, init, exit, evts) \
	static __thread struct evcs_module g_##name##_module_settings = {#name, false, init, exit, NULL, evts, NULL };

#define EVCS_MODULE_CARRY(name, data)						\
	do {									\
		g_##name##_module_settings.argv = malloc(sizeof(*(data)));	\
		memcpy(g_##name##_module_settings.argv, data, sizeof(*(data)));	\
	} while (0)

#define EVCS_MODULE_MOUNT(name)										  \
	do {												  \
		struct evcs_module      *temp = NULL;							  \
		struct evcs_module      *last = g_module_root;						  \
		struct evcs_module      *plus = &g_##name##_module_settings;				  \
		if (NULL == g_module_root) {								  \
			g_module_root = plus;								  \
		} else {										  \
			for (temp = last; temp != NULL && temp != plus; last = temp, temp = temp->next) { \
			}										  \
			if (NULL == temp) {								  \
				last->next = plus;							  \
			}										  \
		}											  \
	} while (0)

#define EVCS_MODULE_ENTRY(name, vary)							      \
	do {										      \
		if (true == vary) {							      \
			if (false == g_##name##_module_settings.state) {		      \
				printf("\t\tmodule "#name " init\n");			      \
				g_##name##_module_settings.init(&g_##name##_module_settings); \
			}								      \
		} else {								      \
			if (true == g_##name##_module_settings.state) {			      \
				printf("\t\tmodule "#name " exit\n");			      \
				g_##name##_module_settings.exit(&g_##name##_module_settings); \
			}								      \
		}									      \
		g_##name##_module_settings.state = vary;				      \
	} while (0)

// if not __thread set, need copy settings

#define EVCS_MODULE_TOUCH(type)						      \
	do {								      \
		struct evcs_module *work = NULL;			      \
		for (work = g_module_root; work != NULL; work = work->next) { \
			if (true == work->state) {			      \
				event_work(work->evts, type, work->argv);     \
			}						      \
		}							      \
	} while (0)

#define EVCS_MODULE_START()			    \
	do {					    \
		EVCS_MODULE_TOUCH(EVCS_EVENT_ROOT); \
	} while (0)

#define EVCS_MODULE_RESTART() ""
// TODO DO EVCS_MODULE_START loop

#ifdef __cplusplus
}
#endif

