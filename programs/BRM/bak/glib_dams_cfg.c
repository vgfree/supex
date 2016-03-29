#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "glib_dams_cfg.h"

const ExceptT EXCEPT_GLIB_EXCEPT = {
	"an except occurred during using glib.",
	EXCEPT_GLIB_EXCEPT_CODE
};

static void init_dams_cfg(struct dams_cfg_file *p_cfg)
{
	assert(p_cfg);

	memset(p_cfg, 0, sizeof(*p_cfg));
	memset(p_cfg->fresh, NO_SET_UP, DIM(p_cfg->fresh));
	memset(p_cfg->delay, NO_SET_UP, DIM(p_cfg->delay));
}

static void free_dams_cfg(struct dams_cfg_file *p_cfg)
{
	int i = 0;

	assert(p_cfg);

	for (i = 0; i < DIM(p_cfg->links) - 1; i++) {
		Free(p_cfg->links[i].host);
	}
}

static void copy_dams_cfg(struct dams_cfg_file *dest, struct dams_cfg_file *src)
{
	int i = 0;

	assert(dest);
	assert(src);

	for (i = 0; i < DIM(dest->links) - 1; i++) {
		dest->links[i].host = x_strdup(src->links[i].host);
		dest->links[i].port = src->links[i].port;
	}

	memcpy(dest->fresh, src->fresh, sizeof(src->fresh));
	memcpy(dest->delay, src->delay, sizeof(src->delay));
}

void parse_glib_conf_file_name(int argc, char **argv, struct config_file *config)
{
	int     opt = 0;
	bool    usedefcfgfile = false;

	SetProgName(argv[0]);
	GetProgName(config->program_name, sizeof(config->program_name));

	while ((opt = getopt(argc, argv, ":c:")) != -1) {
		switch (opt)
		{
			case 'c':
				usedefcfgfile = true;
				snprintf(config->conf_name, sizeof(config->conf_name), "%s", optarg);
				break;

			case ':':
				x_printf(E, "option `%c` need argument", optopt);
				x_printf(E, "USE LIKE :\t./%s -c <config>", config->program_name);
				break;

			default:
				x_printf(E, "unknow option :%c", opt);
				x_printf(E, "USE LIKE \t./%s -c <config>", config->program_name);
		}
	}

	if (!usedefcfgfile) {
		snprintf(config->conf_name, sizeof(config->conf_name), "%s_conf.ini", config->program_name);
	}
}

bool glib_read_dams_cfg(int argc, char **argv, struct dams_cfg_file *volatile p_cfg, struct config_file *config)
{
	GKeyFile *volatile      gkfile = NULL;
	gchar **volatile        str_list = NULL;
	gint *volatile          ivalue_list = NULL;
	GError *volatile        gerror = NULL;

	gchar           *str_val = NULL;
	gboolean        gret = FALSE;
	gsize           length = 0;
	gint            ivalue = 0;

	bool                    ret = false;
	int                     i = 0;
	struct dams_cfg_file    oldcfg = {};

	assert(p_cfg);
	assert(argc > 0);
	assert(argv && argv[0]);

	copy_dams_cfg(&oldcfg, (struct dams_cfg_file *)p_cfg);
	init_dams_cfg((struct dams_cfg_file *)p_cfg);

	TRY
	{
		gkfile = g_key_file_new();
		AssertError(gkfile, ENOMEM);

		g_clear_error((GError **)&gerror);

		parse_glib_conf_file_name(argc, argv, config);

		gret = g_key_file_load_from_file((GKeyFile *)gkfile, config->conf_name, 0, (GError **)&gerror);
		AssertRaise(gret, EXCEPT_GLIB_EXCEPT);

		/********************************* port and host *******************************/
		str_list = g_key_file_get_string_list((GKeyFile *)gkfile, "LINK", "links", &length, (GError **)&gerror);

		if (unlikely(gerror)) {
			x_printf(E, "can't found [links ].");
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		p_cfg->count = length;

		if (p_cfg->count > DIM(p_cfg->links)) {
			x_printf(E, "the number of [links] is too much.");
			RAISE(EXCEPT_ASSERT);
		}

		for (i = 0; i < length; i++) {
			str_val = g_key_file_get_string((GKeyFile *)gkfile, str_list[i], "host", (GError **)&gerror);

			if (unlikely(gerror)) {
				x_printf(E, "can't found [host] that is member of [links]");
				g_clear_error((GError **)&gerror);
				RAISE(EXCEPT_GLIB_EXCEPT);
			}

			p_cfg->links[i].host = x_strdup(str_val);

			ivalue = g_key_file_get_integer((GKeyFile *)gkfile, str_list[i], "port", (GError **)&gerror);

			if (unlikely(gerror)) {
				x_printf(E, "can't found [port] that is member of [links]");
				g_clear_error((GError **)&gerror);
				RAISE(EXCEPT_GLIB_EXCEPT);
			}

			p_cfg->links[i].port = ivalue;
		}

		/************************************ fresh **************************************/
		ivalue_list = g_key_file_get_integer_list((GKeyFile *)gkfile, "LINK", "fresh", &length, (GError **)&gerror);

		if (unlikely(gerror)) {
			x_printf(E, "can't found [fresh].");
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		if ((length > p_cfg->count) || (length > DIM(p_cfg->fresh))) {
			x_printf(E, "the member of [fresh] is too much");
			RAISE(EXCEPT_ASSERT);
		}

		for (i = 0; i < length; i++) {
			p_cfg->fresh[ivalue_list[i]] = IS_SET_UP;
		}

		/************************************ delay **************************************/
		ivalue_list = g_key_file_get_integer_list((GKeyFile *)gkfile, "LINKS", "delay", &length, (GError **)&gerror);

		if (unlikely(gerror)) {
			x_printf(E, "can't found [ delay ].");
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		if ((length > p_cfg->count) || (length > DIM(p_cfg->delay))) {
			x_printf(E, "the member of [delay] is too much .");
			RAISE(EXCEPT_ASSERT);
		}

		for (i = 0; i < length; i++) {
			if ((ivalue_list[i] > p_cfg->count) || (ivalue_list[i] > DIM(p_cfg->delay))) {
				x_printf(E, "the valuse of index that indicate which [ links ] in [delay ].");
				RAISE(EXCEPT_ASSERT);
			}

			p_cfg->delay[ivalue_list[i]] = IS_SET_UP;
		}

		ret = true;
	}
	CATCH
	{
		free_dams_cfg((struct dams_cfg_file *)p_cfg);
		copy_dams_cfg((struct dams_cfg_file *)p_cfg, &oldcfg);
		x_printf(E, "invalid dams config file: %s", config->conf_name);
	}
	FINALLY
	{
		free_dams_cfg(&oldcfg);
		g_key_file_free((GKeyFile *)gkfile);
		g_strfreev((gchar **)str_list);
		g_free((void *)ivalue_list);
	}
	END;

	return ret;
}

