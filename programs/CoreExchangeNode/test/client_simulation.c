#include "communication.h"
#include "comm_message_operator.h"
#include "core_exchange_node_test.h"
//#include "loger.h"

#include <pthread.h>

static int connectfd = 0;
struct comm_context *g_ctx = NULL;
void *client_thread_read(void *usr)
{
  while (1) {
    struct comm_message msg = {};
    init_msg(&msg);
    //printf("start send client.\n");
    comm_recv(g_ctx, &msg, true, -1);
    int size = 0;
    char *frame = get_msg_frame(0, &msg, &size);
    char *buf = (char *)malloc((size + 1) * sizeof(char));
    memset(buf, 0, size + 1);
    memcpy(buf, frame, size);
    printf("%s\n", buf);
    free(buf);
    destroy_msg(&msg);
  }
  return NULL;
}

int test_simulate_client()
{
  g_ctx = comm_ctx_create(EPOLL_SIZE);
  struct cbinfo callback_info = {};
  callback_info.callback = NULL;
  connectfd = comm_socket(g_ctx, "127.0.0.1", "8082", &callback_info, COMM_CONNECT);
  printf("connectfd:%d\n", connectfd);
  if (connectfd == -1) {
    printf("connect error.");
    return -1;
  }
  pthread_t tid;
  assert(pthread_create(&tid, NULL, client_thread_read, NULL) == 0);
  char str[1024];
  while (fgets(str, 1024, stdin) != NULL) {
    struct comm_message msg = {};
    init_msg(&msg);
    set_msg_fd(&msg, connectfd);
    set_msg_frame(0, &msg, strlen(str), str);
    comm_send(g_ctx, &msg, true, -1);
    destroy_msg(&msg);
  }
  return 0;
}
