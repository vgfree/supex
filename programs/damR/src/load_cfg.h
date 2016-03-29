//
//  load_cfg.h
//  supex
//
//  Created by 周凯 on 15/9/16.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef _load_cfg_h
#define _load_cfg_h

#include "data_model.h"
__BEGIN_DECLS

/* --------             */

void load_cfg(int argc, char **argv);

cJSON *load_cfg2json(const char *file);

bool load_json2cfg(cJSON *json, struct cfg *cfg);

void destroy_cfg(struct cfg *cfg);

/* --------             */

__END_DECLS
#endif	/* _load_cfg_h */

