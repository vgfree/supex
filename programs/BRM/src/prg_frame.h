//
//  prg_frame.h
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef prg_frame_h
#define prg_frame_h

#include <stdio.h>
#include "data_model.h"
#include "libmini.h"

__BEGIN_DECLS

/*初始化连接池*/
bool cntpool_init(struct allcfg *cfg);

/*初始化框架，启动，停止，结束回收*/
void initialize(int argc, char **argv);

void run();

void stop();

void finally();

__END_DECLS
#endif	/* prg_frame_h */

