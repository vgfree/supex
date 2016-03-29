//
//  parse_cfg.c
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "supex.h"
#include "parse_cfg.h"

#define PPP_QUEUE_MINSIZE       (1000)
#define PPP_QUEUE_MAXSIZE       (1000000)

static bool _load_hostentry_from_json(cJSON *json, struct hostentry *entry);

static bool _load_hostgrp_from_json(cJSON *json, const char *name, struct hostgroup *grp);

static bool _load_hostcluster_from_json(cJSON *json, const char *name, struct hostcluster *cluster);

static bool _load_hostclusters_from_json(cJSON *json, const char *name, struct hostclusters *cluster);

static bool _load_routerule_from_json(cJSON *json, const char *name, struct routerule *rule);

static bool _load_datasrc_from_json(cJSON *json, struct datasrc *src);

static bool _load_taghash_from_json(cJSON *json, struct taghash *tag);

static bool _load_baseinfo_from_json(cJSON *json, struct allcfg *cfg);

void load_cfg(int argc, char **argv)
{
	struct supex_argv       cfg = {};
	bool                    flag = false;
	cJSON *volatile         json = NULL;

	TRY
	{
		flag = load_supex_args(&cfg, argc, argv, NULL, NULL, NULL);
		assert(flag);

		g_allcfg.cfgfile = x_strdup(cfg.conf_name);

		json = load_cfg2json(cfg.conf_name);
		assert(json);

		flag = load_json2allcfg(json, &g_allcfg);
		assert(flag);
	}
	FINALLY
	{
		cJSON_Delete(json);
	}
	END;
}

/*解析配置文件到json句柄*/
cJSON *load_cfg2json(const char *cfgfile)
{
	cJSON *volatile json = NULL;
	char *volatile  text = NULL;
	off_t           length = 0;
	int volatile    fd = -1;

	assert(cfgfile && cfgfile[0]);

	TRY
	{
		fd = open(cfgfile, O_RDONLY);
		RAISE_SYS_ERROR(fd);

		length = lseek(fd, 0, SEEK_END);
		RAISE_SYS_ERROR(length);

		AssertError(length > 0, EINVAL);

		NewArray(length, text);
		AssertError(length, ENOMEM);

		lseek(fd, 0, SEEK_SET);

		ssize_t bytes = read(fd, text, length);
		RAISE_SYS_ERROR(bytes);

		json = cJSON_Parse(text);

		if (unlikely(!json)) {
			x_printf(E, "load %s error : `%.20s`", cfgfile, cJSON_GetErrorPtr());
		}
	}
	CATCH
	{
		cJSON_Delete(json);
	}
	FINALLY
	{
		if (fd > -1) {
			close(fd);
		}

		Free(text);
	}
	END;

	return json;
}

/*将json中的数据加载到allcfg中*/
bool load_json2allcfg(cJSON *json, struct allcfg *cfg)
{
	bool volatile ret = false;

	assert(cfg && json);

	TRY
	{
		int i = 0;

		ret = _load_baseinfo_from_json(json, cfg);
		assert(ret);
		/* -------------- host                  */

		ret = _load_hostcluster_from_json(json, "srchost", &cfg->host.srchost);
		assert(ret);

		/*check*/
		for (i = 0; i < cfg->host.srchost.hostgrps; i++) {
			assert(cfg->host.srchost.hostgrp[i].hosts == 2);
		}

		ret = _load_hostgrp_from_json(json, "calhost", &cfg->host.calhost);
		assert(ret);

		ret = _load_hostcluster_from_json(json, "routehost", &cfg->host.routehost);
		assert(ret);

		/* -------------- routerule                  */
		ret = _load_routerule_from_json(json, "routerule", &cfg->routerule);
		assert(ret);

		/*检查路由规则 并为其匹配主机组 */
		for (i = 0; i < cfg->routerule.datasrcs; i++) {
			int             x = 0;
			struct datasrc  *datasrc = &cfg->routerule.datasrc[i];

			for (x = 0; x < datasrc->taghashs; x++) {
				int                     y = 0;
				struct hostcluster      *cluster = &cfg->host.routehost;
				
				x_printf(D, "match %s route host group", datasrc->taghash[x].name);
				for (y = 0; y < cluster->hostgrps; y++) {
					if (strcasecmp(cluster->hostgrp[y].name,
						datasrc->taghash[x].name) == 0) {
						/*保存主机标签*/
						datasrc->taghash[x].hostgrp = &cluster->hostgrp[y];
						break;
					}
				}

				assert(datasrc->taghash[x].hostgrp);
			}
		}
		ret = true;
	}
	CATCH
	{
		ret = false;
		allcfg_destroy(cfg);
	}
	END;
	return ret;
}

void load_reload(struct allcfg *newcfg, struct allcfg *oldcfg)
{
	assert(newcfg && oldcfg);

	Free(oldcfg->logpath);
	oldcfg->logpath = newcfg->logpath;
	newcfg->logpath = NULL;
	oldcfg->loglevel = newcfg->loglevel;
	oldcfg->calculate = newcfg->calculate;
	oldcfg->idlesleep = newcfg->idlesleep;
	oldcfg->ischeckfd = newcfg->ischeckfd;
	
	hostcluster_destroy(&oldcfg->host.srchost);
	memcpy(&oldcfg->host.srchost, &newcfg->host.srchost,
		sizeof(newcfg->host.srchost));

	hostgroup_destroy(&oldcfg->host.calhost);
	memcpy(&oldcfg->host.calhost, &newcfg->host.calhost,
		sizeof(newcfg->host.calhost));

	hostcluster_destroy(&oldcfg->host.routehost);
	memcpy(&oldcfg->host.routehost, &newcfg->host.routehost,
		sizeof(newcfg->host.routehost));

	routerule_destroy(&oldcfg->routerule);
	memcpy(&oldcfg->routerule, &newcfg->routerule,
		sizeof(newcfg->routerule));
	
	Free(newcfg->cfgfile);
}

static bool _load_baseinfo_from_json(cJSON *json, struct allcfg *cfg)
{
	cJSON   *obj = NULL;
	cJSON   *ptr = NULL;

	/* -------------- log configure       */
	obj = cJSON_GetObjectItem(json, "log");
	assert(obj && obj->type == cJSON_Object);

	ptr = cJSON_GetObjectItem(obj, "path");
	assert(ptr && ptr->type == cJSON_String);
	assert(ptr->valuestring[0] && g_ProgName);

	size_t size = strlen(ptr->valuestring) + strlen(g_ProgName) + 10;
	NewArray(size, cfg->logpath);
	AssertError(cfg->logpath, ENOMEM);
	snprintf(cfg->logpath, size, "%s/%s.log", ptr->valuestring, g_ProgName);

	ptr = cJSON_GetObjectItem(obj, "level");
	assert(ptr && ptr->type == cJSON_Number);
	cfg->loglevel = ptr->valueint;

	/* --------------  configure       */

	obj = cJSON_GetObjectItem(json, "threads");
	assert(obj && obj->type == cJSON_Number);
	cfg->threads = obj->valueint;
	assert(cfg->threads > 0);

	/* --------------  configure       */

	obj = cJSON_GetObjectItem(json, "paralleltasks");
	assert(obj && obj->type == cJSON_Number);
	cfg->paralleltasks = obj->valueint;
	assert(cfg->paralleltasks > 0);

	/*debug*/
	cfg->paralleltasks *= cfg->threads;
	/* --------------  configure       */

	obj = cJSON_GetObjectItem(json, "packagesize");
	assert(obj && obj->type == cJSON_Number);
	cfg->pkgsize = obj->valueint;
	assert(cfg->pkgsize > 0);

	/* -------------- queue of data cache configure       */
	obj = cJSON_GetObjectItem(json, "queue");
	assert(obj && obj->type == cJSON_Object);

	ptr = cJSON_GetObjectItem(obj, "size");
	assert(ptr && ptr->type == cJSON_Number);

	cfg->queuesize = INRANGE(ptr->valueint,
			PPP_QUEUE_MINSIZE,
			PPP_QUEUE_MAXSIZE);
	
	
	/* ------------- Do or not do calculate				*/
	obj = cJSON_GetObjectItem(json, "calculate");
	if (obj) {
		assert(obj->type == cJSON_True || obj->type == cJSON_False);
		cfg->calculate = obj->valueint;
	} else {
		cfg->calculate = true;
	}
	
	/* ------------- idle sleep 						*/
	obj = cJSON_GetObjectItem(json, "idlesleep");
	if (obj) {
		assert(obj->type == cJSON_Number);
		cfg->idlesleep = obj->valueint;
	} else {
		cfg->idlesleep = 10;
	}
	/* ------------- check fd 						*/
	obj = cJSON_GetObjectItem(json, "checksocket");
	if (obj) {
		assert(obj->type == cJSON_True || obj->type == cJSON_False);
		cfg->ischeckfd = obj->valueint;
	} else {
		cfg->ischeckfd = true;
	}
	return true;
}

static bool _load_hostentry_from_json(cJSON *json, struct hostentry *entry)
{
	bool volatile ret = false;

	assert(entry && json && json->type == cJSON_Object);

	TRY
	{
		cJSON *ptr = NULL;

		/* --------------       */
		ptr = cJSON_GetObjectItem(json, "name");

		if (ptr) {
			assert(ptr->type == cJSON_String);

			entry->name = x_strdup(ptr->valuestring);
			assert(entry->name);
		}

		/* --------------       */
		ptr = cJSON_GetObjectItem(json, "ip");
		assert(ptr && ptr->type == cJSON_String);

		entry->ip = x_strdup(ptr->valuestring);
		assert(entry->ip);
		/* --------------       */
		ptr = cJSON_GetObjectItem(json, "port");
		assert(ptr && ptr->type == cJSON_Number);

		entry->port = ptr->valueint;
		assert(entry->port > 0);

		/* --------------       */
		ptr = cJSON_GetObjectItem(json, "proto");

		if (ptr) {
			assert(ptr->type == cJSON_String);

			if (strcasecmp(ptr->valuestring, "http") == 0) {
				entry->proto = PROTO_TYPE_HTTP;
				/*可以指定url*/
				cJSON *url = NULL;
				url = cJSON_GetObjectItem(json, "url");

				if (url) {
					assert(url->type = cJSON_String);
					entry->url = x_strdup(url->valuestring);
				}
			} else if (strcasecmp(ptr->valuestring, "redis") == 0) {
				entry->proto = PROTO_TYPE_REDIS;
			} else {
				RAISE_SYS_ERROR_ERRNO(ENOPROTOOPT);
			}
		}

		ret = true;
	}
	CATCH
	{
		hostentry_destroy((struct hostentry *)entry);
	}
	END;

	return ret;
}

static bool _load_hostgrp_from_json(cJSON *json, const char *name, struct hostgroup *grp)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && name && grp);

	TRY
	{
		cJSON   *obj = NULL;
		int     size = 0;
		int     i = 0;

		obj = cJSON_GetObjectItem(json, name);

		if (unlikely(!obj || (obj->type != cJSON_Array))) {
			x_printf(E, "not found `%s`", name);
			RAISE(EXCEPT_ASSERT);
		}

		size = cJSON_GetArraySize(obj);
		assert(size > 0);

		grp->name = x_strdup(name);
		AssertError(grp->name, ENOMEM);
		grp->hosts = size;
		NewArray0(size, grp->host);
		AssertError(grp->host, ENOMEM);

		for (i = 0; i < size; i++) {
			cJSON *next = cJSON_GetArrayItem(obj, i);
			ret = _load_hostentry_from_json(next, &grp->host[i]);
			assert(ret);
		}
	}
	CATCH
	{
		hostgroup_destroy(grp);
	}
	END;

	return ret;
}

static bool _load_hostcluster_from_json(cJSON *json, const char *name, struct hostcluster *cluster)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && name && cluster);

	TRY
	{
		cJSON   *obj = NULL;
		int     size = 0;
		int     i = 0;

		obj = cJSON_GetObjectItem(json, name);

		if (unlikely(!obj || (obj->type != cJSON_Object))) {
			x_printf(E, "not found `%s`", name);
			RAISE(EXCEPT_ASSERT);
		}

		size = cJSON_GetArraySize(obj);
		assert(size > 0);

		cluster->name = x_strdup(name);
		AssertError(cluster->name, ENOMEM);
		cluster->hostgrps = size;
		NewArray0(size, cluster->hostgrp);
		AssertError(cluster->hostgrp, ENOMEM);

		for (i = 0; i < size; i++) {
			cJSON *next = cJSON_GetArrayItem(obj, i);
			ret = _load_hostgrp_from_json(obj, next->string, &cluster->hostgrp[i]);
			assert(ret);
		}
	}
	CATCH
	{
		hostcluster_destroy(cluster);
	}
	END;

	return ret;
}

static bool _load_hostclusters_from_json(cJSON *json, const char *name, struct hostclusters *clusters)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && name && clusters);

	TRY
	{
		cJSON   *obj = NULL;
		int     size = 0;
		int     i = 0;

		obj = cJSON_GetObjectItem(json, name);

		if (unlikely(!obj || (obj->type != cJSON_Object))) {
			x_printf(E, "not found `%s`", name);
			RAISE(EXCEPT_ASSERT);
		}

		size = cJSON_GetArraySize(obj);
		assert(size > 0);

		clusters->name = x_strdup(name);
		AssertError(clusters->name, ENOMEM);
		clusters->hostclusters = size;
		NewArray0(size, clusters->hostcluster);
		AssertError(clusters->hostclusters, ENOMEM);

		for (i = 0; i < size; i++) {
			cJSON *next = cJSON_GetArrayItem(obj, i);
			ret = _load_hostcluster_from_json(obj, next->string, &clusters->hostcluster[i]);
			assert(ret);
		}
	}
	CATCH
	{
		hostclusters_destroy(clusters);
	}
	END;

	return ret;
}

static bool _load_routerule_from_json(cJSON *json, const char *name, struct routerule *rule)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && name && rule);

	TRY
	{
		cJSON   *obj = NULL;
		int     size = 0;
		int     i = 0;

		obj = cJSON_GetObjectItem(json, name);

		if (unlikely(!obj || (obj->type != cJSON_Object))) {
			x_printf(E, "not found `%s`", name);
			RAISE(EXCEPT_ASSERT);
		}

		size = cJSON_GetArraySize(obj);
		assert(size > 0);

		rule->name = x_strdup(name);
		AssertError(rule->name, ENOMEM);

		rule->datasrcs = size;
		NewArray0(size, rule->datasrc);
		AssertError(rule->datasrc, ENOMEM);

		for (i = 0; i < size; i++) {
			cJSON *ptr = NULL;
			ptr = cJSON_GetArrayItem(obj, i);
			ret = _load_datasrc_from_json(ptr, &rule->datasrc[i]);
			assert(ret);
		}
	}
	CATCH
	{
		routerule_destroy(rule);
	}
	END;

	return ret;
}

static bool _load_datasrc_from_json(cJSON *json, struct datasrc *src)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && src);

	TRY
	{
		int     size = 0;
		int     i = 0;

		size = cJSON_GetArraySize(json);
		assert(size > 0);

		src->name = x_strdup(json->string);
		AssertError(src->name, ENOMEM);

		src->taghashs = size;

		NewArray0(size, src->taghash);
		AssertError(src->taghash, ENOMEM);

		for (i = 0; i < size; i++) {
			cJSON *obj = NULL;

			obj = cJSON_GetArrayItem(json, i);
			ret = _load_taghash_from_json(obj, &src->taghash[i]);
			assert(ret);
		}
	}
	CATCH
	{
		datasrc_destroy(src);
	}
	END;

	return ret;
}

static bool _load_taghash_from_json(cJSON *json, struct taghash *tag)
{
	bool volatile ret = false;

	assert(json && json->type == cJSON_Object && tag);

	TRY
	{
		cJSON *ptr = NULL;

		tag->name = x_strdup(json->string);
		AssertError(tag->name, ENOMEM);

		ptr = cJSON_GetObjectItem(json, "tag");
		assert(ptr && ptr->type == cJSON_Number);
		tag->tag = ptr->valueint;

		ptr = cJSON_GetObjectItem(json, "hash");
		if (ptr) {
			assert(ptr->type == cJSON_String);
			
			/*parse hash function*/
			if (strcasecmp(ptr->valuestring, "HashTime33") == 0) {
				tag->hash = HashTime33;
			} else if (strcasecmp(ptr->valuestring, "HashFlower") == 0) {
				tag->hash = HashFlower;
			} else if (strcasecmp(ptr->valuestring, "HashReduceBit") == 0) {
				tag->hash = HashReduceBit;
			} else {
				tag->hash = HashPJW;
			}
		} else {
			tag->hash = HashPJW;
		}

		ret = true;
	}
	CATCH
	{
		taghash_destroy(tag);
	}
	END;

	return ret;
}

