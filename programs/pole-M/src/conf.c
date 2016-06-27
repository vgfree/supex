#include "base/utils.h"

#include "conf.h"
#include "json.h"

void config_init(struct pole_conf *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "event_worker_counts", &obj)) {
		p_cfg->event_worker_counts = json_object_get_int(obj);
		x_printf(I, "event_worker_counts: %d", p_cfg->event_worker_counts);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "event_tasker_counts", &obj)) {
		p_cfg->event_tasker_counts = json_object_get_int(obj);
		x_printf(I, "event_tasker_counts: %d", p_cfg->event_tasker_counts);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "max_records", &obj)) {
		p_cfg->max_records = json_object_get_int(obj);
		x_printf(I, "max_records: %d", p_cfg->max_records);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "input_uri", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->input_uri = x_strdup(str_val);
		x_printf(I, "input_uri: %s", p_cfg->input_uri);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "bind_uri", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->bind_uri = x_strdup(str_val);
		x_printf(I, "bind_uri: %s", p_cfg->bind_uri);
	} else { goto fail; }

	return;

fail:
	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}

