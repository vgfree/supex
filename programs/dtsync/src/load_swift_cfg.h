#pragma once

#include "libevcs.h"

bool load_swift_cfg_file(struct swift_cfg_file *p_cfg, char *name);

bool reload_swift_cfg_file(struct swift_cfg_file *p_cfg, const char *filename);

