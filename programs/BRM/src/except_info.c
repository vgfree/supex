//
//  except_info.c
//  supex
//
//  Created by 周凯 on 15/10/31.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "except_info.h"

const ExceptT EXCEPT_CALDATA_FAIL = {
	"calculate fail",
	0
};

const ExceptT EXCEPT_RCVDATA_FAIL = {
	"receive data fail, maybe can't match the format of data",
	0
};

const ExceptT EXCEPT_MATCHINFO_FAIL = {
	"receive data can't the infomation about configure file.",
	0
};

const ExceptT EXCEPT_ROTDATA_FAIL = {
	"route data fail",
	0
};

const ExceptT EXCEPT_NOTNEED_ROTDATA = {
	"not need to route this data.",
};

const ExceptT EXCEPT_LOADCFG_FAIL = {
	"can't load configure file.",
};

