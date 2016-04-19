#include "core_exchange_node_test.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static char expect_value[25] = {0x04, 0x17, 0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a,
  0x00, 0x01, 0x01, 0x00, 0x7f, 0x00, 0x00, 0x01, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02
};

static char login_authentication_package[17] = {0x00, 0x11, 0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a,
  0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 结束端message_to为0x02, 0个数据包。
};

int test_simulate_client()
{
  struct sockaddr_in server_addr;
  int err;
  char server_ip[50] = "127.0.0.1\0";
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    printf("client: create socket error\n");
	return -1;
  }
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8082);
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
  err = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if (err == 0) {
    printf("\nclient: connect to server success. \n start to send login authentication package----------------\n");
    write(socket_fd, login_authentication_package, 17);
    sleep(5);
    printf("start to send data.\n");
    write(socket_fd, expect_value, 25);
  }
  else {
    printf("client: connect error\n");
    close(socket_fd);
    return -1;
  }
  sleep(3);
  close(socket_fd);
  return 0;
}
