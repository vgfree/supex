#ifndef _BUSI_DUMP_H_
#define _BUSI_DUMP_H_

#include "register.h"

struct dump_t
{
	/* MySQL localhost dump. */
	char    dump_host[32];	/* Common user need -h command, and -h192.168.11.27 succeed, -hlocalhost fail.
				 *   User root don't need -h command, if execute mysqldump on local host. */
	char    dump_user[32];	/* Execute mysqldump command's username. */
	char    dump_pwd[32];	/* Execute mysqldump command's password. */
	char    dump_path[128];	/* The destination dumped file path. */
};

static struct dump_t g_dump;

int dump_init(const sync_conf_t *args)
{
	assert(args != NULL);

	memset(&g_dump, 0, sizeof(struct dump_t));
	strcpy(g_dump.dump_host, args->dump_host);
	strcpy(g_dump.dump_user, args->dump_user);
	strcpy(g_dump.dump_pwd, args->dump_pwd);
	strcpy(g_dump.dump_path, args->dump_path);

	return 0;
}

/* Dump current MySQL base data. */
int dump_db(void *args, size_t arg_size)
{
#if 0
	/* args coule be NULL.*/
	char dump[128];

	if (!strlen(g_dump.dump_path)) {
		sprintf(g_dump.dump_path, "./");
	}

	sprintf(dump, "mysqldump -h%s -u%s -p%s -A > %s/basedump.sql 2>&1",
		g_dump.dump_host, g_dump.dump_user, g_dump.dump_pwd, g_dump.dump_path);

	if (_system(dump) != 0) {
		char fpath[256];
		sprintf(fpath, "%s/basedump.sql", g_dump.dump_path);
		FILE *fp = fopen(fpath, "r");

		if (fp != NULL) {
			fread(error, size - 1, 1, fp);
			error[size - 1] = '\0';
			x_printf(E, "%s Execute fail. Error-%s.\n", dump, error);
			fclose(fp);
		}

		return BUSI_ERR_FAIL;
	}
	x_printf(I, "Dump MySQL database Succeed. Path: '%s/basedump.sql'.", g_dump.dump_path);
#endif	/* if 0 */
	return 0;
}
#endif	/* ifndef _BUSI_DUMP_H_ */

