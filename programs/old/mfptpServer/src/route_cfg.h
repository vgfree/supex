#pragma once

#define MAX_LINK_INDEX 32

struct route_link
{
	short   port;
	char    *host;
};

struct route_cfg_file
{
	int                     exp_count;
	int                     inp_count;
	struct route_link       exports[MAX_LINK_INDEX];
	struct route_link       inports[MAX_LINK_INDEX];
};

void read_route_cfg(struct route_cfg_file *p_cfg, char *name);

