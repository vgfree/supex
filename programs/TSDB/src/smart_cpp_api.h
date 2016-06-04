#pragma once

#define DECLARE_SMART_LDB_FUNC(name) int smart_ldb_##name(void *user, union virtual_system **VMS, struct adopt_task_node *task);

DECLARE_SMART_LDB_FUNC(set)

DECLARE_SMART_LDB_FUNC(del)

DECLARE_SMART_LDB_FUNC(mset)

DECLARE_SMART_LDB_FUNC(get)

DECLARE_SMART_LDB_FUNC(lrange)

DECLARE_SMART_LDB_FUNC(keys)

DECLARE_SMART_LDB_FUNC(values)

DECLARE_SMART_LDB_FUNC(info)

DECLARE_SMART_LDB_FUNC(ping)

DECLARE_SMART_LDB_FUNC(exists)

DECLARE_SMART_LDB_FUNC(syncset)

DECLARE_SMART_LDB_FUNC(syncdel)

DECLARE_SMART_LDB_FUNC(compact)

