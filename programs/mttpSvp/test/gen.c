#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DO_FILE		"data"
#define DO_DATA		"good"

void main(void)
{
	char head[2] = {0x10,0x20};
	char size[4] = {};
	int fd = open(DO_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {  
		fprintf(stderr, "Open %s\n", DO_FILE);  
		return;  
	}  
	int bytes = write(fd, head, 2);
	short *temp = (short *)size;
	*(temp) = strlen(DO_DATA) << 16;
	bytes = write(fd, size, 4);
	bytes = write(fd, DO_DATA, strlen(DO_DATA));
	close(fd);
}
