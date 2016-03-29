#include "parser.h"
#include "clog.h"
#include "json.h"
#include <string.h>
#include <assert.h>

int parse_config(sync_conf_t *conf, const char *conf_file_path)
{
	assert(conf != NULL);
	assert(conf_file_path != NULL);

	struct json_object *sub_obj, *root_obj;

	root_obj = json_object_from_file(conf_file_path);
	assert(root_obj != NULL);

	/* Pole-S Log Configure. */
	if (!json_object_object_get_ex(root_obj, "LOG_FILE_PATH", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->log_file_path, json_object_get_string(sub_obj));
	json_object_put(sub_obj);
#if 0

	/*
	 *   if (!json_object_object_get_ex(root_obj, "LOG_FILE_NAME", &sub_obj)) {
	 *        goto PARSE_FAIL;
	 *   }
	 *
	 *   strcpy(conf->log_file_name, json_object_get_string(sub_obj));
	 * */
#endif
	char loglevel[12];

	if (!json_object_object_get_ex(root_obj, "LOG_LEVEL", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(loglevel, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!strcmp(loglevel, "LOG_DEBUG")) {
		conf->log_level = LOG_DEBUG;
	} else if (!strcmp(loglevel, "LOG_INFO")) {
		conf->log_level = LOG_INFO;
	} else if (!strcmp(loglevel, "LOG_WARN")) {
		conf->log_level = LOG_WARN;
	} else if (!strcmp(loglevel, "LOG_ERROR")) {
		conf->log_level = LOG_ERROR;
	} else if (!strcmp(loglevel, "LOG_SYS")) {
		conf->log_level = LOG_SYS;
	} else if (!strcmp(loglevel, "LOG_FATAL")) {
		conf->log_level = LOG_FATAL;
	}

#if 0

	/*
	 *   if (!json_object_object_get_ex(root_obj, "LOG_COUNT", &sub_obj)) {
	 *        goto PARSE_FAIL;
	 *   }
	 *
	 *   conf->log_count = json_object_get_int(sub_obj);
	 *
	 *   if (!json_object_object_get_ex(root_obj, "LOG_MAX_SIZE", &sub_obj)) {
	 *        goto PARSE_FAIL;
	 *   }
	 *
	 *   conf->log_max_size = json_object_get_int(sub_obj);
	 * */
#endif

	/* Network communication with Server. */
	if (!json_object_object_get_ex(root_obj, "SLAVE_UNIQUE_ID", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->id, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "SLAVE_CONN_URI", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->conn_uri, json_object_get_string(sub_obj));

	/* Network operation. */
	if (!json_object_object_get_ex(root_obj, "RUNNING_TYPE", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->run_type, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DEST_DUMP_ID", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dump_id, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	/* MySQL localhost dump. */
	if (!json_object_object_get_ex(root_obj, "DUMP_HOSTNAME", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dump_host, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DUMP_USERNAME", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dump_user, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DUMP_PASSWORD", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dump_pwd, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DUMP_FILEPATH", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dump_path, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	/* Business for Database Operation. */
	if (!json_object_object_get_ex(root_obj, "DB_CONN_STR", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->db_conn_str, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DB_USERNAME", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->db_user, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DB_PASSWORD", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->db_pwd, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	/* Business for writing data to file. */
	if (!json_object_object_get_ex(root_obj, "DT_FILEPATH", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dt_filepath, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	if (!json_object_object_get_ex(root_obj, "DT_FILENAME", &sub_obj)) {
		goto PARSE_FAIL;
	}

	strcpy(conf->dt_filename, json_object_get_string(sub_obj));
	json_object_put(sub_obj);

	json_object_put(root_obj);
	return 0;

PARSE_FAIL:
	json_object_put(root_obj);
	return -1;
}

