#include "Live.h"
#include <iostream>

static Live_CallbacksPtr sCallbacks = NULL;

bool _stublibrary_main(void *inPrivateArgs, Live_DispatchFuncPtr inDispatchFunc)
{
	Live_PrivateArgsPtr theArgs = (Live_PrivateArgsPtr)inPrivateArgs;

	sCallbacks = theArgs->inCallbacks;
	theArgs->outDispatchFunction = inDispatchFunc;
	return true;
}

bool Live_AddRole(Live_Role inRole)
{
	return (sCallbacks->addr[kAddRoleCallback])(inRole);
}

