//
//  prg_frame.h
//  supex
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef prg_frame_h
#define prg_frame_h

#include <stdio.h>
#include "libmini.h"

__BEGIN_DECLS

void init(int argc, char **argv);

void run();

void stop();

void finally();

__END_DECLS
#endif	/* prg_frame_h */

