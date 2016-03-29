#ifndef __LIVE_H__
#define __LIVE_H__

typedef int Live_Role;
typedef void *Live_ServerObject;
typedef void *Live_PrefsObject;
typedef void *Live_ModuleObject;

typedef struct
{
	char outModuleName[256];
} Live_Register_Params;

typedef struct
{
	Live_ServerObject       inServer;
	Live_PrefsObject        inPrefs;
	Live_ModuleObject       inModule;
} Live_Initialize_Params;

typedef union
{
	Live_Register_Params    regParams;
	Live_Initialize_Params  initParams;
} Live_RoleParam, *Live_RoleParamPtr;

typedef bool (*Live_DispatchFuncPtr)(Live_Role inRole, Live_RoleParamPtr inParamBlock);

typedef bool (*Live_CallbackProcPtr)(Live_Role inRole);

enum
{
	kNewCallback = 0,
	kAddRoleCallback = 1,
	kAddAttributeCallback = 2,
	kLastCallback = 5
};

enum
{
	Live_Register_Role = 1,
	Live_Initialize_Role = 2,
	Live_Read_Role = 3,
	Live_Write_Role = 4
};

typedef struct
{
	Live_CallbackProcPtr    addr[kLastCallback];
	int                     role[kLastCallback];
} Live_Callbacks, *Live_CallbacksPtr;

typedef struct
{
	int                     live;
	Live_CallbacksPtr       inCallbacks;
	Live_DispatchFuncPtr    outDispatchFunction;
} Live_PrivateArgs, *Live_PrivateArgsPtr;

bool Live_AddRole(Live_Role inRole);

bool _stublibrary_main(void *inPrivateArgs, Live_DispatchFuncPtr inDispatchFunc);
#endif	// __LIVE_H__

