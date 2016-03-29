#include "LiveModule.h"
#include "LiveRelayModule.h"
#include "Live.h"
#include <cstring>

bool Read();

bool Write();

bool LiveRelayModuleDispatch(Live_Role inRole, Live_RoleParamPtr inParams);

bool Register(Live_Register_Params *inParams);

bool Initialize(Live_Initialize_Params *inParams);

bool Write();

bool LiveRelayModule_Main(void *inPrivateArgs)
{
	return _stublibrary_main(inPrivateArgs, LiveRelayModuleDispatch);
}

bool LiveRelayModuleDispatch(Live_Role inRole, Live_RoleParamPtr inParams)
{
	switch (inRole)
	{
		case Live_Register_Role:
			return Register(&inParams->regParams);

		case Live_Initialize_Role:
			return Initialize(&inParams->initParams);

		case Live_Read_Role:
			return Read();

		case Live_Write_Role:
			return Write();
	}
	return true;
}

bool Register(Live_Register_Params *inParams)
{
	(void)Live_AddRole(Live_Initialize_Role);
	(void)Live_AddRole(Live_Read_Role);
	(void)Live_AddRole(Live_Write_Role);

	const char *sModuleName = "LiveRelayModule";
	::strcpy(inParams->outModuleName, (const char *)sModuleName);
	return true;
}

bool Initialize(Live_Initialize_Params *inParams)
{
	std::cout << "LiveRelayModule: Initialize<<<----\n";

	return true;
}

bool Read()
{
	std::cout << "LiveRelayModule: please input something<<<---\n";

	return true;
}

bool Write()
{
	std::cout << "LiveRelayModule: Write<<<----\n";

	return true;
}

