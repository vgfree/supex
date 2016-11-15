/**
 * @file   module.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Apr 15 21:58:11 2012
 *
 * @brief  Header file for proto http module.
 *
 *
 */

#ifndef RINOO_MODULE_PROTO_HTTP_H_
#define RINOO_MODULE_PROTO_HTTP_H_

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>

#include "rinoo/debug/module.h"
#include "rinoo/memory/module.h"
#include "rinoo/struct/module.h"
#include "rinoo/scheduler/module.h"
#include "rinoo/net/module.h"

#include "rinoo/proto/http/http_header.h"
#include "rinoo/proto/http/http_request.h"
#include "rinoo/proto/http/http_response.h"
#include "rinoo/proto/http/http.h"
#include "rinoo/proto/http/http_file.h"
#include "rinoo/proto/http/http_easy.h"

#endif /* !RINOO_MODULE_PROTO_HTTP_H_ */
