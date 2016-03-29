#ifndef __GLIB_LOAD_SWIFT_CFG_H_
#define __GLIB_LOAD_SWIFT_CFG_H_

#include "swift_cfg.h"
#include "glib_dams_cfg.h"

bool glib_load_swift_cfg_file(int argc, char **argv, struct swift_cfg_file *p_cfg, struct config_file *config);

bool glib_reload_swift_cfg_file(int argc, char **argv, struct swift_cfg_file *volatile p_cfg, struct config_file *config);
#endif

