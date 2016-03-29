#ifndef __LiveServer_H__
#define __LiveServer_H__
#include "Live.h"
#include "LiveModule.h"

extern LiveModule *moduleQueue[];

class LiveServer {
public:
	LiveServer();
	~LiveServer();
	bool Initialize();

	bool InitModule(bool inInitialState);

	bool AddModule(LiveModule *inModule);

protected:
	bool LoadModules(char *routeOfDynamicModules);

	bool LoadCompiledInModules();

private:
	static Live_Callbacks sCallbacks;
};
#endif	// __LiveServer_H__

