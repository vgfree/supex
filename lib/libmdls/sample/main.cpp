#include <iostream>
#include <unistd.h>
#include "Live.h"
#include "LiveServer.h"
#include "LiveModule.h"

LiveModule      *moduleQueue[26];
LiveServer      *sServer = NULL;

bool StartServer(char *theConfigFilePath);

void RunServer();

int main(int argc, char *argv[])
{
	char *theConfigFilePath = NULL;

	if (StartServer(theConfigFilePath)) {
		RunServer();
	}

	return 0;
}

bool StartServer(char *theConfigFilePath)
{
	bool inInitialState = false;

	sServer = new LiveServer();
	sServer->Initialize();
	bool result = sServer->InitModule(inInitialState);
	return result;
}

void RunServer()
{
	int                     i;
	Live_RoleParamPtr       inParams = new Live_RoleParam;	// must have a addr
	int                     role = 0;

	do {
		std::cout << "please input role(1-4):";
		std::cin >> role;
		switch (role)
		{
			case Live_Register_Role:

				for (i = 0; moduleQueue[i] != NULL; i++) {
					bool err = (moduleQueue[i]->GetDispatch())(Live_Register_Role, inParams);

					if (err == true) {
						break;
					}
				}

				break;

			case Live_Initialize_Role:

				for (i = 0; moduleQueue[i] != NULL; i++) {
					bool err = (moduleQueue[i]->GetDispatch())(Live_Initialize_Role, inParams);

					if (err == true) {
						break;
					}
				}

				break;

			case Live_Read_Role:

				for (i = 0; moduleQueue[i] != NULL; i++) {
					bool err = (moduleQueue[i]->GetDispatch())(Live_Read_Role, inParams);

					if (err == true) {
						break;
					}
				}

				break;

			case Live_Write_Role:

				for (i = 0; moduleQueue[i] != NULL; i++) {
					bool err = (moduleQueue[i]->GetDispatch())(Live_Write_Role, inParams);

					if (err == true) {
						break;
					}
				}

				break;

			default:
				std::cout << "no the role";
				break;
		}
	} while (1);
}

