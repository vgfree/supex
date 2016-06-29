#include <string.h>
#include <assert.h>

#include "conf.h"
#include "json.h"
#include "libmini.h"

void config_init(struct pole_conf *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	/* Pole-S Log Configure. */
	if (json_object_object_get_ex(cfg, "LOG_FILE", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->log_file, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "LOG_LEVEL", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else { goto fail; }

	/* Network communication with Server. */
	if (json_object_object_get_ex(cfg, "SLAVE_UNIQUE_ID", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->self_uid, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "SLAVE_CONN_URI", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->conn_uri, str_val);
	} else { goto fail; }

	/* Network operation. */
	if (json_object_object_get_ex(cfg, "RUNNING_TYPE", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->run_type, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DEST_DUMP_UID", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->dump_uid, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "SELF_INCR_SEQ", &obj)) {
		p_cfg->incr_seq = json_object_get_int64(obj);
	} else { goto fail; }

	/* MySQL localhost dump. */
	if (json_object_object_get_ex(cfg, "DUMP_HOSTNAME", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->dump_host, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DUMP_USERNAME", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->dump_user, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DUMP_PASSWORD", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->dump_pwd, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DUMP_FILEPATH", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->dump_path, str_val);
	} else { goto fail; }

	/* Business for Database Operation. */
	if (json_object_object_get_ex(cfg, "DB_CONN_STR", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->db_conn_str, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DB_USERNAME", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->db_user, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "DB_PASSWORD", &obj)) {
		str_val = json_object_get_string(obj);
		strcpy(p_cfg->db_pwd, str_val);
	} else { goto fail; }

	json_object_put(cfg);
	return;

fail:
	x_printf(E, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}

