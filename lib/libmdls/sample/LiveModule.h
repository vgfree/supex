#ifndef __LiveModule_H__
#define __LiveModule_H__
#include "Live.h"
#include <iostream>

static Live_CallbacksPtr sCallbacks;
typedef bool (*Live_MainEntryPointPtr)(void *inPrivateArgs);

class LiveModule {
public:
	LiveModule(const std::string & moduleName, char *inPath = NULL);
	~LiveModule();
	bool SetupModule(Live_CallbacksPtr inCallbacks, Live_MainEntryPointPtr inEntrypoint = NULL);

	bool CallDispatch(Live_Role inRole, Live_RoleParamPtr inParams)
	{
		return (fDispatchFunc)(inRole, inParams);
	}

	bool AddRole(Live_Role inRole)
	{
		return (sCallbacks->addr[kAddRoleCallback])(inRole);
	}

	bool Live_AddAttribute(Live_Role inRole)
	{
		return (sCallbacks->addr[kAddAttributeCallback])(inRole);
	}

	const std::string GetModuleName();

	Live_CallbacksPtr GetCallbacks();

	Live_DispatchFuncPtr GetDispatch();

private:
	Live_DispatchFuncPtr    fDispatchFunc;
	const std::string       module;
};
#endif	// __LiveModule_H__

