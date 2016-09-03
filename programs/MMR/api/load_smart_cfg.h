#pragma once

#include "major/smart_cfg.h"

bool load_smart_cfg_argv(int opt, void *user);

void load_smart_cfg_file(struct smart_cfg_file *p_cfg, char *name);

