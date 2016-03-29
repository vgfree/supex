//
//  data_model.c
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "pool_api.h"
#include "data_model.h"

struct allcfg g_allcfg = {};

struct framentry g_framentry = {
	.stat   = FRAME_STAT_NONE,
};

static inline bool _hostentry_connect(struct hostentry *host, int poolsize);

void hostentry_destroy(struct hostentry *host)
{
	return_if_fail(host);
	Free((host)->name);
	Free((host)->ip);
	Free((host)->hostaddr);
	Free((host)->url);
}

void hostgroup_destroy(struct hostgroup *grp)
{
	return_if_fail(grp);
	int i = 0;

	for (i = 0; grp->host && i < grp->hosts; i++)
		hostentry_destroy(&grp->host[i]);

	Free(grp->name);
}

void hostcluster_destroy(struct hostcluster *cluster)
{
	return_if_fail(cluster);
	int i = 0;

	for (i = 0; cluster->hostgrp && i < cluster->hostgrps; i++)
		hostgroup_destroy(&cluster->hostgrp[i]);

	Free(cluster->name);
}

void hostclusters_destroy(struct hostclusters *clusters)
{
	return_if_fail(clusters);
	int i = 0;

	for (i = 0; clusters->hostcluster && i < clusters->hostclusters; i++)
		hostcluster_destroy(&clusters->hostcluster[i]);

	Free(clusters->name);
}

void allcfg_destroy(struct allcfg *cfg)
{
	return_if_fail(cfg);

	hostcluster_destroy(&cfg->host.srchost);
	hostgroup_destroy(&cfg->host.calhost);
	hostcluster_destroy(&cfg->host.routehost);

	routerule_destroy(&cfg->routerule);

	Free(cfg->logpath);
	Free(cfg->cfgfile);
}

bool hostgroup_connect(struct hostgroup *grp, int poolsize)
{
	int     i = 0;
	bool    flag = true;

	assert(grp);

	x_printf(D, "connect the host group of `%s`", grp->name);

	for (i = 0; i < grp->hosts; i++) {
		bool r = false;
		r = _hostentry_connect(&grp->host[i], poolsize);
		if (unlikely(!r)) flag = false;
	}

	return flag;
}

bool hostcluster_connect(struct hostcluster *cluster, int poolsize)
{
	int     i = 0;
	bool    flag = true;

	assert(cluster);

	x_printf(D, "connect the host cluster of `%s`", cluster->name);

	for (i = 0; i < cluster->hostgrps; i++) {
		bool r = false;
		r = hostgroup_connect(&cluster->hostgrp[i], poolsize);
//		return_val_if_fail(flag, false);
		if (unlikely(!r)) flag = false;
	}

	return flag;
}

bool hostclusters_connect(struct hostclusters *clusters, int poolsize)
{
	int     i = 0;
	bool    flag = true;

	assert(clusters);

	x_printf(D, "connect the host clusters of `%s`", clusters->name);

	for (i = 0; i < clusters->hostclusters; i++) {
		bool r = false;
		r = hostcluster_connect(&clusters->hostcluster[i], poolsize);
		if (unlikely(!r)) flag = false;
	}

	return flag;
}

void netdata_destroy(struct netdata *data, bool distcnt)
{
	return_if_fail(data);

	if (likely(distcnt && data->cntpool && (data->fd > -1))) close((int)data->fd);

	if (likely(data->json)) cJSON_Delete(data->json);
	cache_clean(&data->cache);
	cache_finally(&data->cache);
}

void taskdata_free(struct taskdata **pdata, bool distcnt)
{
	int i = 0;

	assert(pdata);

	return_if_fail(*pdata);

	// free source network data
	netdata_destroy(&(*pdata)->src, distcnt);

	// free calculate data
	for (i = 0; (*pdata)->caldata && i < (*pdata)->caldatas; i++)
		netdata_destroy(&(*pdata)->caldata[i], distcnt);

	// free route data
	for (i = 0; (*pdata)->rtdata && i < (*pdata)->rtdatas; i++)
		netdata_destroy(&(*pdata)->rtdata[i], distcnt);

	Free((*pdata)->rthost);
	Free((*pdata)->caldata);
	Free((*pdata)->rtdata);
	Free(*pdata);
}

void routerule_destroy(struct routerule *rule)
{
	return_if_fail(rule);
	int i = 0;

	for (i = 0; rule->datasrc && i < rule->datasrcs; i++)
		datasrc_destroy(&rule->datasrc[i]);

	Free(rule->name);
}

void datasrc_destroy(struct datasrc *src)
{
	return_if_fail(src);
	int i = 0;

	for (i = 0; src->taghash && i < src->taghashs; i++)
		taghash_destroy(&src->taghash[i]);

	Free(src->name);
}

void taghash_destroy(struct taghash *tag)
{
	return_if_fail(tag);
	Free(tag->name);
}

void procentry_stop(struct procentry *proc)
{
	return_if_fail(proc);
	ATOMIC_SET((int *)&proc->stat, PROC_STAT_STOP);
	ATOMIC_SET(&proc->frame->stat, FRAME_STAT_STOP);
}

/* --------             */
static inline bool _hostentry_connect(struct hostentry *host, int poolsize)
{
	bool ret = false;

	assert(host);

	x_printf(D, "connect to `%s:%d`", host->ip, host->port);

	ret = pool_api_init(host->ip, host->port, poolsize, false);

	if (unlikely(!ret)) x_perror("connect to `%s:%d` fail.", host->ip, host->port);

	return ret;
}
