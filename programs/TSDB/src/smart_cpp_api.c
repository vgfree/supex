#include "major_def.h"
#include "smart_api.h"
#include "smart_task.h"

#include "tsdb_api.h"

#define DEFINE_SMART_LDB_FUNC(name)						      \
	int smart_ldb_##name(void *user, void *task)				      \
	{									      \
		struct smart_task_node  *smart_task = (struct smart_task_node *)task; \
		struct data_node        *p_node = get_pool_addr(smart_task->sfd);     \
										      \
		return tsdb_cmd_##name(p_node);					      \
	}

DEFINE_SMART_LDB_FUNC(set)

DEFINE_SMART_LDB_FUNC(del)

DEFINE_SMART_LDB_FUNC(mset)

DEFINE_SMART_LDB_FUNC(get)

DEFINE_SMART_LDB_FUNC(lrange)

DEFINE_SMART_LDB_FUNC(keys)

DEFINE_SMART_LDB_FUNC(values)

DEFINE_SMART_LDB_FUNC(info)

DEFINE_SMART_LDB_FUNC(ping)

DEFINE_SMART_LDB_FUNC(exists)

DEFINE_SMART_LDB_FUNC(syncset)

DEFINE_SMART_LDB_FUNC(syncdel)

DEFINE_SMART_LDB_FUNC(compact)

