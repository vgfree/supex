#include <unistd.h>
#include <assert.h>

#include "load_smart_cfg.h"
#include "load_swift_cfg.h"

#include "major/smart_api.h"
#include "major/swift_api.h"

#include "smart_cpp_api.h"
#include "swift_cpp_api.h"

#include "tsdb_cfg.h"
#include "tsdb_entry.h"

#define DEFINE_LDB_CMD_LIST(name)					   \
	static struct cmd_list s_##name##_cmd_list[] = {		   \
		{ SET_FUNC_ORDER,     (TASK_VMS_FCB)name##_ldb_set     }, \
		{ DEL_FUNC_ORDER,     (TASK_VMS_FCB)name##_ldb_del     }, \
		{ MSET_FUNC_ORDER,    (TASK_VMS_FCB)name##_ldb_mset    }, \
		{ GET_FUNC_ORDER,     (TASK_VMS_FCB)name##_ldb_get     }, \
		{ LRANGE_FUNC_ORDER,  (TASK_VMS_FCB)name##_ldb_lrange  }, \
		{ KEYS_FUNC_ORDER,    (TASK_VMS_FCB)name##_ldb_keys    }, \
		{ VALUES_FUNC_ORDER,  (TASK_VMS_FCB)name##_ldb_values  }, \
		{ INFO_FUNC_ORDER,    (TASK_VMS_FCB)name##_ldb_info    }, \
		{ PING_FUNC_ORDER,    (TASK_VMS_FCB)name##_ldb_ping    }, \
		{ EXISTS_FUNC_ORDER,  (TASK_VMS_FCB)name##_ldb_exists  }, \
		{ SYNCSET_FUNC_ORDER, (TASK_VMS_FCB)name##_ldb_syncset }, \
		{ SYNCDEL_FUNC_ORDER, (TASK_VMS_FCB)name##_ldb_syncdel }, \
		{ COMPACT_FUNC_ORDER, (TASK_VMS_FCB)name##_ldb_compact }  \
	}

DEFINE_LDB_CMD_LIST(smart);
DEFINE_LDB_CMD_LIST(swift);

struct smart_cfg_list   g_smart_cfg_list = {};
struct swift_cfg_list   g_swift_cfg_list = {};

struct tsdb_cfg_file g_tsdb_cfg_file = {};

extern unsigned int g_max_req_size;

static void load_smart_cfg_func(struct smart_cfg_func *p_func_info, struct cmd_list *p_cmd_list, int cmd_count)
{
	int i = 0;

	for (i = 0; i < cmd_count; ++i) {
		p_func_info[(int)p_cmd_list[i].type].type = BIT8_TASK_TYPE_ALONE;
		p_func_info[(int)p_cmd_list[i].type].func = (TASK_VMS_FCB)p_cmd_list[i].func;
	}
}

static void load_swift_cfg_func(struct swift_cfg_func *p_func_info, struct cmd_list *p_cmd_list, int cmd_count)
{
	int i = 0;

	for (i = 0; i < cmd_count; ++i) {
		p_func_info[(int)p_cmd_list[i].type].type = BIT8_TASK_TYPE_ALONE;
		p_func_info[(int)p_cmd_list[i].type].func = (TASK_VMS_FCB)p_cmd_list[i].func;
	}
}

static void *start_smart(void *arg)
{
	struct safe_init_step *info = arg;

	SAFE_PTHREAD_INIT_COME(info);
	SAFE_PTHREAD_INIT_OVER(info);

	smart_start();

	return NULL;
}

static void swift_entry_init(void)
{
	g_max_req_size = 64 * 1024 * 1024;
	tsdb_entry_init();
}

static void swift_shut_down(void)
{
	tsdb_shut_down();
}

static void smart_entry_init(void)
{
	g_max_req_size = 64 * 1024 * 1024;
}

int main(int argc, char *argv[])
{
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
	load_swift_cfg_func(g_swift_cfg_list.func_info, s_swift_cmd_list, sizeof(s_swift_cmd_list) / sizeof(s_swift_cmd_list[0]));

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.shut_down = swift_shut_down;

	swift_mount(&g_swift_cfg_list);

	read_tsdb_cfg(&g_tsdb_cfg_file, g_swift_cfg_list.argv_info.conf_name);

	snprintf(g_smart_cfg_list.argv_info.conf_name,
		sizeof(g_smart_cfg_list.argv_info.conf_name),
		"%s", g_swift_cfg_list.argv_info.conf_name);

	snprintf(g_smart_cfg_list.argv_info.serv_name,
		sizeof(g_smart_cfg_list.argv_info.serv_name),
		"%s", g_swift_cfg_list.argv_info.serv_name);

	snprintf(g_smart_cfg_list.argv_info.msmq_name,
		sizeof(g_smart_cfg_list.argv_info.msmq_name),
		"%s", g_swift_cfg_list.argv_info.msmq_name);

	load_smart_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);
	load_smart_cfg_func(g_smart_cfg_list.func_info, s_smart_cmd_list, sizeof(s_smart_cmd_list) / sizeof(s_smart_cmd_list[0]));

	g_smart_cfg_list.entry_init = smart_entry_init;

	smart_mount(&g_smart_cfg_list);

	safe_start_pthread((void *)start_smart, 1, NULL, NULL);

	swift_start();

	return 0;
}

