//
//  except_info.h
//  supex
//
//  Created by 周凯 on 15/10/31.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef except_info_h
#define except_info_h

#include <stdio.h>
#include "libmini.h"

__BEGIN_DECLS
extern const ExceptT    EXCEPT_CALDATA_FAIL;
extern const ExceptT    EXCEPT_RCVDATA_FAIL;
extern const ExceptT    EXCEPT_MATCHINFO_FAIL;
extern const ExceptT    EXCEPT_ROTDATA_FAIL;
extern const ExceptT    EXCEPT_NOTNEED_ROTDATA;
extern const ExceptT    EXCEPT_LOADCFG_FAIL;
__END_DECLS
#endif	/* except_info_h */

