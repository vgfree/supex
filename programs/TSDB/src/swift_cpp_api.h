#pragma once

#define DECLARE_SWIFT_LDB_FUNC(name) int swift_ldb_##name(void *user, union virtual_system **VMS, struct adopt_task_node *task);

DECLARE_SWIFT_LDB_FUNC(set)

DECLARE_SWIFT_LDB_FUNC(del)

DECLARE_SWIFT_LDB_FUNC(mset)

DECLARE_SWIFT_LDB_FUNC(get)

DECLARE_SWIFT_LDB_FUNC(sadd)

DECLARE_SWIFT_LDB_FUNC(lrange)

DECLARE_SWIFT_LDB_FUNC(keys)

DECLARE_SWIFT_LDB_FUNC(values)

DECLARE_SWIFT_LDB_FUNC(info)

DECLARE_SWIFT_LDB_FUNC(ping)

DECLARE_SWIFT_LDB_FUNC(exists)

DECLARE_SWIFT_LDB_FUNC(syncset)

DECLARE_SWIFT_LDB_FUNC(syncdel)

DECLARE_SWIFT_LDB_FUNC(compact)

