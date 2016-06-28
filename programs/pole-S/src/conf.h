#pragma once
#include <stdint.h>

struct pole_conf
{
	/* Pole-S Log Configure. */
	char    log_file[128];
	int     log_level;	// LOG_DEBUG|LOG_INFO|LOG_WARN|LOG_ERROR|LOG_FATAL

	/* Network Communication. */
	char    self_uid[32];		/* Client Unique ID. */
	char    conn_uri[32];	/* ZeroMQ type's connect e.g "tcp://192.168.1.2:8686" */

	/* Client running type. */
	char    run_type[10];	/* "DUMP" or "INCR" */
	char    dump_uid[32];	/* The destination node's ID, It will be dumped,
				 *   it is used only for run_type is "DUMP". */
	uint64_t  incr_seq;
	/* MySQL localhost dump. */
	char    dump_host[32];	/* Common user need -h command, and -h192.168.11.27 succeed, -hlocalhost fail.
				 *   User root don't need -h command, if execute mysqldump on local host. */
	char    dump_user[32];	/* Execute mysqldump command's username. */
	char    dump_pwd[32];	/* Execute mysqldump command's password. */
	char    dump_path[128];	/* The destination dumped file path. 
				   Dumped file full path, the path should be exist and can be read and write.*/

	/* Business for Database. */
	char    db_conn_str[32];	/* DB connect string, "192.168.11.27:1521/mysql" */
	char    db_user[24];		/* DB user name. */
	char    db_pwd[32];		/* DB password. */
};

void config_init(struct pole_conf *p_cfg, char *name);

