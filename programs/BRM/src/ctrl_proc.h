//
//  ctrl_proc.h
//  supex
//
//  Created by 周凯 on 15/10/29.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef ctrl_proc_h
#define ctrl_proc_h

#include "libmini.h"

__BEGIN_DECLS

/*
 * 控制进程，会话通信，重载配置文件等
 */
void main_loop(struct framentry *frame);

__END_DECLS
#endif	/* ctrl_proc_h */

