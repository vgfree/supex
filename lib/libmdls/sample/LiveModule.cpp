#include "LiveModule.h"

static bool Addr(Live_Role inRole);

LiveModule::LiveModule(const std::string &moduleName, char *inPath) : fDispatchFunc(NULL), module(moduleName)
{
	sCallbacks = new Live_Callbacks;
	sCallbacks->addr[kAddRoleCallback] = &Addr;
}

LiveModule::~LiveModule()
{}

bool LiveModule::SetupModule(Live_CallbacksPtr inCallbacks, Live_MainEntryPointPtr inEntrypoint)
{
	Live_PrivateArgs thePrivateArgs;

	*inCallbacks = *sCallbacks;
	thePrivateArgs.inCallbacks = inCallbacks;
	thePrivateArgs.outDispatchFunction = NULL;
	(inEntrypoint)(&thePrivateArgs);
	fDispatchFunc = thePrivateArgs.outDispatchFunction;
	return true;
}

const std::string LiveModule::GetModuleName()
{
	return module;
}

bool Addr(Live_Role inRole)
{
	switch (inRole)
	{
		case Live_Register_Role:
			std::cout << "Live_Register_Role\n";
			break;

		case Live_Initialize_Role:
			std::cout << "Live_Initialize_Role\n";
			break;

		case Live_Read_Role:
			std::cout << "Live_Read_Role\n";
			break;

		case Live_Write_Role:
			std::cout << "Live_Write_Role\n";
			break;

		default:
			std::cout << "no the role\n";
			break;
	}
	return true;
}

Live_DispatchFuncPtr LiveModule::GetDispatch()
{
	return fDispatchFunc;
}

Live_CallbacksPtr LiveModule::GetCallbacks()
{
	return sCallbacks;
}

