/**
 * @file   module.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Jan 25 00:27:02 2012
 *
 * @brief  Header file for network module.
 *
 *
 */

#ifndef RINOO_MODULE_NET_H_
#define RINOO_MODULE_NET_H_

#define _GNU_SOURCE

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

#include "rinoo/global/module.h"
#include "rinoo/memory/module.h"
#include "rinoo/struct/module.h"
#include "rinoo/scheduler/module.h"

#include "rinoo/net/socket_class.h"
#include "rinoo/net/socket.h"
#include "rinoo/net/socket_class_tcp.h"
#include "rinoo/net/socket_class_udp.h"
#include "rinoo/net/socket_class_ssl.h"
#include "rinoo/net/tcp.h"
#include "rinoo/net/udp.h"
#include "rinoo/net/ssl.h"

#endif /* !RINOO_MODULE_NET_H_ */
