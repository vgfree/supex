#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <error.h>
#define PORT    6990
#define MSGMAX  500
#define MAX     10
int main()
{
	char                    sendmsg[MSGMAX];
	char                    recvmsg[MSGMAX];
	struct sockaddr_in      server_addr;
	int                     sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket error\n");
		return -1;
	}

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(server_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		printf("connect error\n");
		return -1;
	}

	memset(sendmsg, 0, MSGMAX);
	sendmsg[0] = '#';
	sendmsg[1] = 'M';
	sendmsg[2] = 'F';
	sendmsg[3] = 'P';
	sendmsg[4] = 'T';
	sendmsg[5] = 'P';
	sendmsg[6] = 0x01;
	sendmsg[7] = 0x00;
	sendmsg[8] = 0x03;
	sendmsg[9] = 0x01;
	sendmsg[10] = 0x00;
	sendmsg[11] = 0x10;

	int i = 0;
	sendmsg[12] = '1';
	sendmsg[13] = '0';
	sendmsg[14] = '0';
	sendmsg[15] = '0';
	sendmsg[16] = '0';
	sendmsg[17] = '0';
	sendmsg[18] = '0';
	sendmsg[19] = '0';
	sendmsg[20] = '0';
	sendmsg[21] = '0';
	sendmsg[22] = '0';
	sendmsg[23] = '0';
	sendmsg[24] = '0';
	sendmsg[25] = '0';
	sendmsg[26] = '1';
	sendmsg[27] = 0x01;
	char    buf_gps[1000] = "nt=0&mt=21776&bizid=a4edbef334a89611e595bf984be10d2b68&gps=221215102957,11444.98295E,4045.92141N,262,18,746;221215102958,11444.97898E,4045.92095N,262,19,746;221215102959,11444.97479E,4045.92069N,263,20,746;221215103000,11444.97046E,4045.92029N,262,20,746;221215103001,11444.96605E,4045.91999N,262,21,746&tokencode=0al15UMqpo&imsi=460022211427272&imei=752795632561713&mod=SG900";
	char    *p_gps = buf_gps;
	int     len_gps = strlen(buf_gps);
	printf("%d\n\n", len_gps);

	if (send(sockfd, sendmsg, 28, 0) < 0) {
		printf("send msg failed!");
		close(sockfd);
		return -1;
	}

	memset(recvmsg, 0, MSGMAX);

	int recv_size;

	if ((recv_size = recv(sockfd, recvmsg, sizeof(recvmsg), 0)) < 0) {
		printf("\nserver has no reply!\n");
		close(sockfd);
		return -1;
	}

	printf("recv_size = %d\n", recv_size);

	for (i = 0; i < 33; i++) {
		if ((0 <= i) && (i <= 5)) {
			printf("%c ", recvmsg[i]);
		} else {
			printf("%d ", recvmsg[i]);
		}
	}

	printf("\n");
	sendmsg[10] = 0x01;
	sendmsg[11] = 0x01;
	sendmsg[12] = 0x78;
	memcpy(sendmsg + 13, buf_gps, 376);

	while (1) {
		if (send(sockfd, sendmsg, 389, 0) < 0) {
			printf("send msg failed!");
			close(sockfd);
			return -1;
		}

		sleep(30);
	}

	close(sockfd);
	return 0;
}

