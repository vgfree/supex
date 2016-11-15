/**
 * @file   module.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Tue Jul 15 19:36:13 2014
 *
 * @brief  Header file for fs module.
 *
 *
 */

#ifndef RINOO_MODULE_FS_H_
#define RINOO_MODULE_FS_H_

#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "rinoo/global/module.h"
#include "rinoo/memory/module.h"
#include "rinoo/struct/module.h"
#include "rinoo/scheduler/module.h"

#include "rinoo/fs/browse.h"
#include "rinoo/fs/inotify.h"

#endif /* !RINOO_MODULE_FS_H_ */
