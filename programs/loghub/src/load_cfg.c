#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_cfg.h"
#include "json.h"

void load_loghub_cfg_argv(struct loghub_cfg_argv *p_cfg, int argc, char **argv)
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
				x_printf(E, "unknow option :%c\n", opt);
				x_printf(E, "USE LIKE :\n\t./%s -c %s_conf.json\n",
					p_cfg->serv_name, p_cfg->serv_name);
				exit(EXIT_FAILURE);
		}
	}

	if (!cfg) {
		/*设置默认配置文件名称*/

		snprintf(p_cfg->conf_name, sizeof(p_cfg->conf_name), "%s_conf.json", p_cfg->serv_name);
	}
}

void load_loghub_cfg_file(struct loghub_cfg_file *p_cfg, char *name)
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

	json_object_put(cfg);
	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}

