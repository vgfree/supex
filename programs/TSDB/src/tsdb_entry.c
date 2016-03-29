#include "misc.h"
#include "zk.h"
#include "utils.h"
#include "slave.h"

#include "tsdb_api.h"
#include "tsdb_cfg.h"

extern struct tsdb_cfg_file g_tsdb_cfg_file;

void tsdb_entry_init(void)
{
	/* check pid file. */
	check_pid_file();

	/* write pid file. */
	write_pid_file();

	/* initial the engine db and start it. */
	if (0 != tsdb_cmd_init(&g_tsdb_cfg_file)) {
		x_printf(F, "failed to initial the tsdb engine.");
		exit(EXIT_FAILURE);
	}

	/* initial and register the data node to zookeeper. */
	if (g_tsdb_cfg_file.node_type == CLUSTER) {
		if (0 != zk_init("Hello zookeeper.")) {
			x_printf(F, "failed to initial zookeeper");
			exit(EXIT_FAILURE);
		}

		if (0 != zk_register_node()) {
			x_printf(F, "failed to register the node to zookeeper.");
			exit(EXIT_FAILURE);
		}
	}
}

void tsdb_shut_down(void)
{
	/* unregister the data node to zookeeper. */
	if (g_tsdb_cfg_file.node_type == CLUSTER) {
		zk_unregister_node();
		zk_close();
	}

	/* close engine. */
	tsdb_cmd_close();

	/* remove pid file. */
	remove_pid_file();

	x_printf(I, "exit tsdb.");
}

