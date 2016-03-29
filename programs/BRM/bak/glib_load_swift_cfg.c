#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <glib.h>

#include "glib_load_swift_cfg.h"

static void init_swift_cfg(struct swift_cfg_file *p_cfg)
{
	assert(p_cfg);
	memset(p_cfg, 0, sizeof(*p_cfg));
}

static void free_swift_cfg(struct swift_cfg_file *p_cfg)
{
	assert(p_cfg);

	Free(p_cfg->log_path);
	Free(p_cfg->log_file);
}

static void copy_swift_cfg(struct swift_cfg_file *dest, struct swift_cfg_file *src)
{
	assert(dest);
	assert(src);

	if (src->log_path) {
		dest->log_path = x_strdup(src->log_path);
	}

	if (src->log_file) {
		dest->log_file = x_strdup(src->log_file);
	}
}

bool glib_load_swift_cfg(int argc, char **argv, struct swift_cfg_file *p_cfg, struct config_file *config)
{
	GKeyFile *volatile      gkfile = NULL;
	gchar **volatile        str_list = NULL;
	GError *volatile        gerror = NULL;

	gchar           *str_value = NULL;
	gboolean        gret = FALSE;
	gint            ivalue = 0;

	bool ret = false;

	assert(p_cfg);
	assert(argc > 0);
	assert(argv && argv[0]);

	init_swift_cfg(p_cfg);

	TRY
	{
		gkfile = g_key_file_new();
		AssertError(gkfile, ENOMEM);

		g_clear_error((GError **)&gerror);

		parse_glib_conf_file_name(argc, argv, config);

		gret = g_key_file_load_from_file((GKeyFile *)gkfile, config->conf_name, 0, (GError **)&gerror);
		AssertRaise(gret, EXCEPT_GLIB_EXCEPT);

		/***************************** swift **********************************/
		ivalue = g_key_file_get_integer((GKeyFile *)gkfile, "SWIFT", "swift_port", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->srv_port = (short)ivalue;
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		ivalue = g_key_file_get_integer((GKeyFile *)gkfile, "SWIFT", "swift_worker_counts", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->worker_counts = (short)ivalue;
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		ivalue = g_key_file_get_integer((GKeyFile *)gkfile, "SWIFT", "max_req_size", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->max_req_size = ivalue;
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		/******************************* log *******************************/
		str_value = g_key_file_get_string((GKeyFile *)gkfile, "LOG", "log_path", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_path = x_strdup(str_value);
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		str_value = g_key_file_get_string((GKeyFile *)gkfile, "LOG", "log_file", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_file = x_strdup(str_value);
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		ivalue = g_key_file_get_integer((GKeyFile *)gkfile, "LOG", "log_level", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_level = (short)ivalue;
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		if (p_cfg->ptype == USE_HTTP_PROTO) {
			/**************************** HTTP ************************************/
			str_value = g_key_file_get_string((GKeyFile *)gkfile, "HTTP", "api_apply", (GError **)&gerror);

			if (likely(!gerror)) {
				p_cfg->api_counts++;
				p_cfg->api_apply = x_strdup(str_value);
			} else {
				g_clear_error((GError **)&gerror);
				RAISE(EXCEPT_GLIB_EXCEPT);
			}

			str_value = g_key_file_get_string((GKeyFile *)gkfile, "HTTP", "api_fetch", (GError **)&gerror);

			if (likely(!gerror)) {
				p_cfg->api_counts++;
				p_cfg->api_fetch = x_strdup(str_value);
			} else {
				g_clear_error((GError **)&gerror);
				RAISE(EXCEPT_GLIB_EXCEPT);
			}

			str_value = g_key_file_get_string((GKeyFile *)gkfile, "HTTP", "api_merge", (GError **)&gerror);

			if (likely(!gerror)) {
				p_cfg->api_counts++;
				p_cfg->api_merge = x_strdup(str_value);
			} else {
				g_clear_error((GError **)&gerror);
				RAISE(EXCEPT_GLIB_EXCEPT);
			}

			int     length = 0;
			int     i = 0;
			int     len = 0;
			str_list = g_key_file_get_string_list((GKeyFile *)gkfile, "HTTP", "api_custom", &length, (GError **)&gerror);

			if (likely(!gerror)) {
				p_cfg->api_counts += length;

				assert(p_cfg->api_counts <= DIM(p_cfg->api_names));

				for (i = 0; i < length; i++) {
					len = snprintf(&p_cfg->api_names[i][0], sizeof(p_cfg->api_names[i]), "%s", str_list[i]);
					assert(len <= sizeof(p_cfg->api_names[i]));
				}
			} else {
				g_clear_error((GError **)&gerror);
			}
		}

		ret = true;
	}
	CATCH
	{
		x_printf(E, "invalid config file :%s", config->conf_name);
	}
	FINALLY
	{
		g_strfreev((gchar **)str_list);
		g_key_file_free((GKeyFile *)gkfile);
	}
	END;

	return ret;
}

bool glib_reload_swift_cfg_file(int argc, char **argv, struct swift_cfg_file *volatile p_cfg, struct config_file *config)
{
	GKeyFile *volatile      gkfile = NULL;
	GError *volatile        gerror = NULL;
	gchar                   *str_value = NULL;
	gboolean                gret = FALSE;
	gint                    ivalue = 0;

	bool                    ret = false;
	struct swift_cfg_file   old = {};

	assert(p_cfg);
	assert(argc > 0);
	assert(argv && argv[0]);

	copy_swift_cfg(&old, (struct swift_cfg_file *)p_cfg);
	free_swift_cfg((struct swift_cfg_file *)p_cfg);

	TRY
	{
		gkfile = g_key_file_new();
		AssertError(gkfile, ENOMEM);

		g_clear_error((GError **)&gerror);

		parse_glib_conf_file_name(argc, argv, config);

		gret = g_key_file_load_from_file((GKeyFile *)gkfile, config->conf_name, 0, (GError **)&gerror);
		AssertRaise(gret, EXCEPT_GLIB_EXCEPT);

		/******************** start reload attribute ***********************/
		str_value = g_key_file_get_string((GKeyFile *)gkfile, "LOG", "log_path", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_path = x_strdup(str_value);
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		str_value = g_key_file_get_string((GKeyFile *)gkfile, "LOG", "log_file", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_file = x_strdup(str_value);
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		ivalue = g_key_file_get_integer((GKeyFile *)gkfile, "LOG", "log_level", (GError **)&gerror);

		if (likely(!gerror)) {
			p_cfg->log_level = (short)ivalue;
		} else {
			g_clear_error((GError **)&gerror);
			RAISE(EXCEPT_GLIB_EXCEPT);
		}

		ret = true;
	}
	CATCH
	{
		free_swift_cfg((struct swift_cfg_file *)p_cfg);
		copy_swift_cfg((struct swift_cfg_file *)p_cfg, &old);
		x_printf(E, "invalid config file : %s", config->conf_name);
	}
	FINALLY
	{
		free_swift_cfg(&old);
		g_key_file_free((GKeyFile *)gkfile);
	}
	END;

	return ret;
}

