#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <zmq.h>

// 0:push, 1:pull, 2:login, 3:API
int main(int argc, char *argv[])
{
  g_ctx = zmq_ctx_new();
  thread_status = malloc(4 * sizeof(pthread_t));
  assert(pthread_create(&thread_status[0], NULL, push_thread, NULL) == 0);
  assert(pthread_create(&thread_status[1], NULL, pull_thread, NULL) == 0);
  assert(pthread_create(&thread_status[2], NULL, login_thread, NULL) == 0);
  assert(pthread_create(&thread_status[3], NULL, api_thread, NULL) == 0);

  void *status;
  for (int i = 0; i < 4; i++) {
    pthread_join(thread_status[i], &status);
  }
  free(thread_status);
  zmq_ctx_destroy(g_ctx);
}
