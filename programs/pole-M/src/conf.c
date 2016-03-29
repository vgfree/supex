#include "conf.h"
#include "json.h"
#include "slog/slog.h"

config_t g_pole_conf;

int config_init(const char *path)
{
	struct json_object *sub_obj, *root_obj;

	root_obj = json_object_from_file(path);

	if (!root_obj) {
		goto dislike;
	}

	if (json_object_object_get_ex(root_obj, "input_uri", &sub_obj)) {
		strncpy(g_pole_conf.input_uri, json_object_get_string(sub_obj), sizeof(g_pole_conf.input_uri));
		json_object_put(sub_obj);

		x_printf(I, "input_uri: %s", g_pole_conf.input_uri);
	} else { goto dislike; }

	if (json_object_object_get_ex(root_obj, "bind_uri", &sub_obj)) {
		strncpy(g_pole_conf.bind_uri, json_object_get_string(sub_obj), sizeof(g_pole_conf.bind_uri));
		json_object_put(sub_obj);

		x_printf(I, "bind_uri: %s", g_pole_conf.bind_uri);
	} else { goto dislike; }

	if (json_object_object_get_ex(root_obj, "log_path", &sub_obj)) {
		strncpy(g_pole_conf.log_path, json_object_get_string(sub_obj), sizeof(g_pole_conf.log_path));
		json_object_put(sub_obj);

		x_printf(I, "log_path: %s", g_pole_conf.log_path);
	} else { goto dislike; }

	if (json_object_object_get_ex(root_obj, "log_level", &sub_obj)) {
		char loglevel[32];
		strncpy(loglevel, json_object_get_string(sub_obj), sizeof(loglevel));
		json_object_put(sub_obj);

		if (!strcmp(loglevel, "LOG_DEBUG")) {
			g_pole_conf.log_level = LOG_DEBUG;
		} else if (!strcmp(loglevel, "LOG_INFO")) {
			g_pole_conf.log_level = LOG_INFO;
		} else if (!strcmp(loglevel, "LOG_WARN")) {
			g_pole_conf.log_level = LOG_WARN;
		} else if (!strcmp(loglevel, "LOG_ERROR")) {
			g_pole_conf.log_level = LOG_ERROR;
		} else if (!strcmp(loglevel, "LOG_SYS")) {
			g_pole_conf.log_level = LOG_SYS;
		} else if (!strcmp(loglevel, "LOG_FATAL")) {
			g_pole_conf.log_level = LOG_FATAL;
		}

		x_printf(I, "log_level: %s:%d", loglevel, g_pole_conf.log_level);
	} else { goto dislike; }

	if (json_object_object_get_ex(root_obj, "max_records", &sub_obj)) {
		g_pole_conf.max_records = json_object_get_int(sub_obj);
		json_object_put(sub_obj);

		x_printf(I, "max_records: %d", g_pole_conf.max_records);
	} else { goto dislike; }

	if (json_object_object_get_ex(root_obj, "thread_number", &sub_obj)) {
		g_pole_conf.thread_number= json_object_get_int(sub_obj);
		json_object_put(sub_obj);

		x_printf(I, "thread_number: %d", g_pole_conf.thread_number);
	} else { goto dislike; }

	return 0;

dislike:
	json_object_put(root_obj);
	return -1;
}

