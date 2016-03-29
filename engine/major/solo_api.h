#pragma once

#include <netinet/in.h>
#include "supex.h"
#include "major_def.h"

/* >>>>>>>>>>>>>>>>>>>>>==configuration==<<<<<<<<<<<<<<<<<<<<<<< */

struct solo_cfg_file
{
	int     srv_port;

	int     ptype;	/* http or redis */

	char    *log_path;
	char    *log_file;
	short   log_level;
};

struct solo_cfg_list
{
	struct supex_argv       cfg_argv;
	struct solo_cfg_file    cfg_file;

	/* just http protocol use */
	struct api_list         cfg_http_apis[MAX_API_COUNTS];

	void                    (*entry_init)(void);
	void                    (*idle_work)(void);
} g_cfg_list;

/* >>>>>>>>>>>>>>>>>>>>>>>=============<<<<<<<<<<<<<<<<<<<<<<<<< */

/* >>>>>>>>>>>>>>>>>>>>>>==glabe value==<<<<<<<<<<<<<<<<<<<<<<<< */
struct ev_loop *g_p_loop;

/* >>>>>>>>>>>>>>>>>>>>>>===============<<<<<<<<<<<<<<<<<<<<<<<< */

/* >>>>>>>>>>>>>>>>>>>>>>===function===<<<<<<<<<<<<<<<<<<<<<<<<< */

void solo_mount(void);

int solo_start(void);

/* >>>>>>>>>>>>>>>>>>>>>>===============<<<<<<<<<<<<<<<<<<<<<<<< */

