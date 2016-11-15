/**
 * @file   module.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Dec 28 00:12:30 2009
 *
 * @brief  Header file for scheduler module.
 *
 *
 */

#ifndef RINOO_MODULE_SCHEDULER_H_
#define RINOO_MODULE_SCHEDULER_H_

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "rinoo/debug/module.h"
#include "rinoo/global/module.h"
#include "rinoo/struct/module.h"

#include "rinoo/scheduler/fcontext.h"
#include "rinoo/scheduler/task.h"
#include "rinoo/scheduler/node.h"
#include "rinoo/scheduler/epoll.h"
#include "rinoo/scheduler/spawn.h"
#include "rinoo/scheduler/scheduler.h"
#include "rinoo/scheduler/channel.h"

#endif /* !RINOO_MODULE_SCHEDULER_H_ */
