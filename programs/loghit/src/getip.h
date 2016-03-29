#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <sys/vfs.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void get_ip_write_idpub(char *name)
{
	struct ifaddrs  *ifAddrStruct = NULL;
	void            *tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct != NULL) {
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {	// check it is IP4
									// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

			// check is not 127.0.0.1
			if (strcmp(addressBuffer, "127.0.0.1") != 0) {
				printf("%s\n", addressBuffer);
				FILE *fp;
				fp = fopen(name, "w");

				if (fp) {
					fputs(addressBuffer, fp);
					fclose(fp);
				}
			}
		}

		ifAddrStruct = ifAddrStruct->ifa_next;
	}
}

