#ifndef _BUSI_DB_H_
#define _BUSI_DB_H_

/* Function: Execute SQL (INSERT|UPDATE|DELETE) in MySQL database. */

#include "register.h"

#include <mysql/mysql.h>
#include <assert.h>

static MYSQL *g_pMysql = NULL;

int db_connect(struct pole_conf *conf)
{
	assert(conf != NULL);

	/* INIT the MYSQL structure. */
	g_pMysql = mysql_init(NULL);

	if (!g_pMysql) {
		x_printf(E, "MySQL Handle initialize fail.\n");
		return -1;
	}

	int     port;
	char    host[52];
	char    *s, *db, str[128];

	strcpy(str, conf->db_conn_str);
	strcpy(host, strtok_r(str, ":", &s));
	port = atoi(strtok_r(s, "/", &db));
	x_printf(I, "Parsing the db_connstr succeed. host:<%s> port:%d dbname:<%s>\n", host, port, db);

	if (mysql_real_connect(g_pMysql, host, conf->db_user, conf->db_pwd, db, port, NULL, 0) == NULL) {
		x_printf(E, "Connect to database fail. Error-%s.\n", mysql_error(g_pMysql));
		mysql_close(g_pMysql);
		return -1;
	}

	x_printf(I, "Connect to database succeed.\n");

	return 0;
}

int db_exec(char *error, size_t size, void *args, size_t arg_size)
{
	assert(args != NULL);

	if (mysql_query(g_pMysql, (char *)args) != 0) {
		x_printf(E, "mysql_query: Execute (%s) fail. Error-%s.\n", (char *)args, mysql_error(g_pMysql));
		int res, errnbr = mysql_errno(g_pMysql);

		if (errnbr == 321) {		// xxx || error == xxx || error == xxx)
			res = BUSI_ERR_FATAL;	// serious error.
		} else {
			res = BUSI_ERR_FAIL;	// Common error;
		}

		strncpy(error, mysql_error(g_pMysql), size - 1);
		error[size - 1] = '\0';
		return res;
	}

	return 0;
}

void db_disconnect()
{
	if (g_pMysql) {
		mysql_close(g_pMysql);
	}
}
#endif	/* ifndef _BUSI_DB_H_ */

