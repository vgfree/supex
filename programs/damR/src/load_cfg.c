//
//  load_cfg.c
//  supex
//
//  Created by 周凯 on 15/9/16.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "supex.h"
#include "load_cfg.h"
/* --------             */
struct cfg g_cfg = {};

static void _load_netcfg_from_json(cJSON *json, char *tag, struct netcfg *cfg);

static void _load_logcfg_from_json(cJSON *json, char *tag, struct logcfg *cfg);

static void _load_queuecfg_from_json(cJSON *json, char *tag, struct queuecfg *cfg);

/* --------             */

void load_cfg(int argc, char **argv)
{
	struct supex_argv       cfg = {};
	bool                    flag = false;
	cJSON *volatile         json = NULL;

	TRY
	{
		flag = load_supex_args(&cfg, argc, argv, NULL, NULL, NULL);
		assert(flag);

		g_cfg.cfgfile = x_strdup(cfg.conf_name);

		json = load_cfg2json(cfg.conf_name);
		assert(json);

		flag = load_json2cfg(json, &g_cfg);
		assert(flag);
	}
	FINALLY
	{
		cJSON_Delete(json);
	}
	END;
}

cJSON *load_cfg2json(const char *file)
{
	cJSON *volatile json = NULL;
	char *volatile  text = NULL;
	int volatile    fd = -1;

	TRY
	{
		size_t size = 0;
		size = FS_GetSize(file);

		NewArray0(size + 1, text);
		AssertError(text, ENOMEM);

		fd = open(file, O_RDONLY);
		RAISE_SYS_ERROR(fd);

		ssize_t bytes = 0;
		bytes = read(fd, text, size);
		assert(bytes == size);

		json = cJSON_Parse(text);

		if (unlikely(!json)) {
			x_printf(E, "load %s error : `%.20s`", file, cJSON_GetErrorPtr());
		}
	}
	CATCH
	{
		cJSON_Delete(json);
		json = NULL;
	}
	FINALLY
	{
		close(fd);
		Free(text);
	}
	END;

	return json;
}

bool load_json2cfg(cJSON *json, struct cfg *cfg)
{
	assert(cfg && json);

	TRY
	{
		_load_logcfg_from_json(json, "log", &cfg->log);
		_load_netcfg_from_json(json, "recv", &cfg->recv);
		_load_netcfg_from_json(json, "send", &cfg->send);
		_load_queuecfg_from_json(json, "queue", &cfg->queue);

		REFOBJ(cfg);
		ReturnValue(true);
	}
	CATCH
	{
		destroy_cfg(cfg);
	}
	END;

	return false;
}

void destroy_cfg(struct cfg *cfg)
{
	return_if_fail(UNREFOBJ(cfg));
	Free(cfg->cfgfile);
	Free(cfg->recv.ip);
	Free(cfg->send.ip);
	Free(cfg->queue.path);
}

static void _load_netcfg_from_json(cJSON *json, char *tag, struct netcfg *cfg)
{
	cJSON   *obj = NULL;
	cJSON   *ptr = NULL;

	assert(cfg && json && tag);

	obj = cJSON_GetObjectItem(json, tag);
	assert(obj && obj->type == cJSON_Object);

	ptr = cJSON_GetObjectItem(obj, "ip");
	assert(ptr && ptr->type == cJSON_String);
	cfg->ip = x_strdup(ptr->valuestring);
	AssertError(cfg->ip, ENOMEM);

	ptr = cJSON_GetObjectItem(obj, "port");
	assert(ptr && ptr->type == cJSON_Number && (ptr->valueint > 0 && ptr->valueint < UINT16_MAX));
	cfg->port = ptr->valueint;

	ptr = cJSON_GetObjectItem(obj, "proto");
	assert(ptr && ptr->type == cJSON_String);

	if (strcasecmp(ptr->valuestring, "zmq") == 0) {
		cfg->proto = PROTO_ZMQ;
	} else if (strcasecmp(ptr->valuestring, "http") == 0) {
		cfg->proto = PROTO_HTTP;
	} else {
		RAISE_SYS_ERROR(EPROTO);
	}

	if (cfg->proto == PROTO_ZMQ) {
		ptr = cJSON_GetObjectItem(obj, "cache");
		assert(ptr && ptr->type == cJSON_Number && ptr->valueint > 0);
		cfg->cache = ptr->valueint;
	}
}

static void _load_logcfg_from_json(cJSON *json, char *tag, struct logcfg *cfg)
{
	cJSON   *obj = NULL;
	cJSON   *ptr = NULL;

	assert(cfg && json && tag);

	obj = cJSON_GetObjectItem(json, tag);
	assert(obj && obj->type == cJSON_Object);

	ptr = cJSON_GetObjectItem(obj, "path");
	assert(ptr && ptr->type == cJSON_String);

	bool flag = false;
	flag = FS_IsDirectory(ptr->valuestring, true);
	AssertRaise(flag, EXCEPT_SYS);

	size_t size = strlen(ptr->valuestring) + strlen(g_ProgName) + 10;

	NewArray(size, cfg->path);
	AssertError(cfg->path, ENOMEM);

	snprintf(cfg->path, size, "./%s/%s.log", ptr->valuestring, g_ProgName);

	ptr = cJSON_GetObjectItem(obj, "level");
	assert(ptr && ptr->type == cJSON_Number);
	cfg->level = ptr->valueint;
}

static void _load_queuecfg_from_json(cJSON *json, char *tag, struct queuecfg *cfg)
{
	cJSON   *obj = NULL;
	cJSON   *ptr = NULL;

	assert(json && cfg && tag);

	obj = cJSON_GetObjectItem(json, tag);
	assert(obj && obj->type == cJSON_Object);

	ptr = cJSON_GetObjectItem(obj, "type");
	assert(ptr && ptr->type == cJSON_String);

	if (strcasecmp(ptr->valuestring, "file") == 0) {
		cfg->type = FILE_QUEUE;
	} else if (strcasecmp(ptr->valuestring, "share") == 0) {
		cfg->type = SHM_QUEUE;
	} else if (strcasecmp(ptr->valuestring, "memory") == 0) {
		cfg->type = MEM_QUEUE;
	} else {
		RAISE(EXCEPT_ASSERT);
	}

	ptr = cJSON_GetObjectItem(obj, "path");
	assert(ptr && ptr->type == cJSON_String);

	bool flag = false;
	flag = FS_IsDirectory(ptr->valuestring, true);
	AssertRaise(flag, EXCEPT_SYS);

	cfg->path = x_strdup(ptr->valuestring);
	AssertError(cfg->path, ENOMEM);

	ptr = cJSON_GetObjectItem(obj, "capacity");
	assert(ptr && ptr->type == cJSON_Number && ptr->valueint > 0);
	cfg->capacity = ptr->valueint;

	ptr = cJSON_GetObjectItem(obj, "cellsize");
	assert(ptr && ptr->type == cJSON_Number && ptr->valueint > 0);
	cfg->cellsize = ptr->valueint;

	ptr = cJSON_GetObjectItem(obj, "seq");
	assert(ptr && ptr->type == cJSON_Number && ptr->valueint > 0);
	cfg->seq = ptr->valueint;
}

