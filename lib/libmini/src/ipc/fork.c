//
//  fork.c
//
//
//  Created by 周凯 on 15/8/28.
//
//

#include "fork.h"
#include "../atomic/atomic.h"

void InitProcess()
{
	AO_SET(&g_ThreadID, -1);
	AO_SET(&g_ProcessID, -1);
#if 1
	AO_SET(&_g_ef_, NULL);
	AO_SET(&g_errno, 0);
#endif
}

