#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_cfg.h"
#include "json.h"

char *x_strdup(const char *src)
{
	if (src == NULL) {
		return NULL;
	}

	int     len = (int)strlen(src);
	char    *out = calloc(len + 1, sizeof(char));
	strcpy(out, src);
	return out;
}

void load_loghit_cfg_argv(struct loghit_cfg_argv *p_cfg, int argc, char **argv)
{
	const char      *ptr = NULL;
	char            opt = '\0';
	short           cfg = false;

	assert(p_cfg);
	assert(argv && argv[0]);

	/*
	 * 存储程序名称和消息队列名称
	 */
	ptr = strrchr(argv[0], '/');
	ptr = ptr ? ptr + 1 : argv[0];
	snprintf(p_cfg->serv_name, sizeof(p_cfg->serv_name), "%s", ptr);

	while ((opt = getopt(argc, argv, "c:h:p")) != -1) {
		switch (opt)
		{
			case 'c':
				cfg = true;
				snprintf(p_cfg->conf_name, sizeof(p_cfg->conf_name), "%s", optarg);
				x_printf(D, "load config:'%s'\n", optarg);
				break;

			default:
				x_printf(E, COLOR_LIGHT_RED "unknow option :%c\n" COLOR_NONE, opt);
				x_printf(E, COLOR_LIGHT_RED "USE LIKE :\n\t./%s -c %s_conf.json\n" COLOR_NONE,
					p_cfg->serv_name, p_cfg->serv_name);
				exit(EXIT_FAILURE);
		}
	}

	if (!cfg) {
		/*设置默认配置文件名称*/

		snprintf(p_cfg->conf_name, sizeof(p_cfg->conf_name), "%s_conf.json", p_cfg->serv_name);
	}
}

void load_loghit_cfg_file(struct loghit_cfg_file *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "loghub_port", &obj)) {
		p_cfg->port = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "loghub_host", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->host = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "unique_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->unique_file = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "space_usleep", &obj)) {
		p_cfg->space_usleep = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "loghitDB_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->loghitDB_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "follow_list", &obj)) {
		int                     i = 0;
		int                     all = json_object_array_length(obj);
		struct json_object      *itr_obj = NULL;
		struct follow_node      **node = &p_cfg->list;

		for (i = 0; i < all; i++) {
			itr_obj = json_object_array_get_idx(obj, i);
			str_val = json_object_get_string(itr_obj);
			*node = calloc(1, sizeof(struct follow_node));
			(*node)->name = x_strdup(str_val);
			node = &(*node)->next;
		}
	}

	json_object_put(cfg);
	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}

