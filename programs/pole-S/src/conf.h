#ifndef _PARSER_H_
#define _PARSER_H_

typedef struct
{
	/* Pole-S Log Configure. */
	char    log_file[128];
	// char    log_file_name[32];
	// int     log_count;	// The max log file's count.
	int     log_level;	// LOG_DEBUG|LOG_INFO|LOG_WARN|LOG_ERROR|LOG_FATAL
	// long    log_max_size;	// The log file max size. Unit: (MB).

	/* Network Communication. */
	char    id[32];		/* Client Unique ID. */
	char    conn_uri[32];	/* ZeroMQ type's connect e.g "tcp://192.168.1.2:8686" */

	/* Client running type. */
	char    run_type[20];	/* "EV_DUMP" or "EV_INCREMENT" */
	char    dump_id[20];	/* The destination node's ID, It will be dumped,
				 *   it is used only for run_type is "EV_DUMP". */
	/* MySQL localhost dump. */
	char    dump_host[32];	/* Common user need -h command, and -h192.168.11.27 succeed, -hlocalhost fail.
				 *   User root don't need -h command, if execute mysqldump on local host. */
	char    dump_user[32];	/* Execute mysqldump command's username. */
	char    dump_pwd[32];	/* Execute mysqldump command's password. */
	char    dump_path[128];	/* The destination dumped file path. */

	/* Business for Database. */
	char    db_conn_str[32];	/* DB connect string, "192.168.11.27:1521/mysql" */
	char    db_user[24];		/* DB user name. */
	char    db_pwd[32];		/* DB password. */

	/* Business for write to file. */
	char    dt_filepath[128];	/* Data store file path. */
	char    dt_filename[32];	/* Data store file name. */
} sync_conf_t;

int parse_config(sync_conf_t *conf, const char *conf_file_path);
#endif	/* ifndef _PARSER_H_ */

