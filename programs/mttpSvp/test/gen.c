#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define DO_FILE "data"
#define DO_DATA "M=dd832a886a5d11e6918c00a0d1eb3984&A=&C=vUAJk58ulblwzElx&T=1472537797&B=300816061633&G=0,112142970,32035900,43,16,58;1,112143010,32035930,44,17,58;2,112143060,32035970,42,19,58;3,112143100,32036020,39,21,58;4,112143160,32036060,41,24,58&TYPE=2&IP=172.16.82.1&PORT=44204"

void main(void)
{
	char    head[3] = { 0x10, 0x00, 0x02 };
	int     fd = open(DO_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		fprintf(stderr, "Open %s\n", DO_FILE);
		return;
	}

	int             bytes = write(fd, head, sizeof(head));
	unsigned int    size = htonl(strlen(DO_DATA));
	bytes = write(fd, (char *)&size, 4);
	bytes = write(fd, DO_DATA, strlen(DO_DATA));
	close(fd);
}

