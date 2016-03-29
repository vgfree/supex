#include "LiveServer.h"
#include <iostream>
#include "LiveRelayModule.h"

Live_Callbacks LiveServer::sCallbacks;

LiveServer::LiveServer() {}

LiveServer::~LiveServer() {}

bool LiveServer::Initialize()
{
	return true;
}

bool LiveServer::InitModule(bool inInitialState)
{
	LoadModules(NULL);
	LoadCompiledInModules();
	return true;
}

bool LiveServer::AddModule(LiveModule *inModule)
{
	int     i = 0;
	bool    err = true;

	for (i = 0; moduleQueue[i] != NULL && i < 26; i++) {}

	moduleQueue[i] = inModule;
	return err;
}

bool LiveServer::LoadModules(char *routeOfDynamicModules)
{
	std::cout << "LoadModules";

	return true;
}

bool LiveServer::LoadCompiledInModules()
{
	LiveModule *theRelayModule = new LiveModule("LiveRelayModule");

	theRelayModule->SetupModule(&sCallbacks, &LiveRelayModule_Main);
	AddModule(theRelayModule);
	return true;
}

