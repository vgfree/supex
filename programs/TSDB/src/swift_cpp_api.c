#include "major_def.h"
#include "swift_api.h"
#include "swift_task.h"

#include "tsdb_api.h"

#define DEFINE_SWIFT_LDB_FUNC(name)						     \
	int swift_ldb_##name(void *W)						     \
	{									     \
		SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W; \
		struct swift_task_node  *swift_task = &p_swift_worker->task;	     \
		struct data_node        *p_node = get_pool_addr(swift_task->sfd);    \
										     \
		return tsdb_cmd_##name(p_node);					     \
	}

DEFINE_SWIFT_LDB_FUNC(set)

DEFINE_SWIFT_LDB_FUNC(del)

DEFINE_SWIFT_LDB_FUNC(mset)

DEFINE_SWIFT_LDB_FUNC(get)

DEFINE_SWIFT_LDB_FUNC(lrange)

DEFINE_SWIFT_LDB_FUNC(keys)

DEFINE_SWIFT_LDB_FUNC(values)

DEFINE_SWIFT_LDB_FUNC(info)

DEFINE_SWIFT_LDB_FUNC(ping)

DEFINE_SWIFT_LDB_FUNC(exists)

DEFINE_SWIFT_LDB_FUNC(syncset)

DEFINE_SWIFT_LDB_FUNC(syncdel)

DEFINE_SWIFT_LDB_FUNC(compact)

