//
//  parse_cfg.h
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef parse_cfg_h
#define parse_cfg_h

#include <stdio.h>
#include "cJSON.h"
#include "libmini.h"
#include "data_model.h"

__BEGIN_DECLS

/*加载数据*/
void load_cfg(int argc, char **argv);

/*解析配置文件到json句柄*/
cJSON *load_cfg2json(const char *cfgfile);

/*将json中的数据加载到allcfg中*/
bool load_json2allcfg(cJSON *json, struct allcfg *cfg);

/*拷贝配置*/
void load_reload(struct allcfg *newcfg, struct allcfg *oldcfg);

__END_DECLS
#endif	/* parse_cfg_h */

