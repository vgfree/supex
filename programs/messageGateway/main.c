#include "http_dispatch.h"
#include "message_concentrator.h"

int main(int argc, char *argv[])
{
  pthread_t tid;
  if (concentrator_init(tid) != 0) {
    printf("concentrator not init.");
	return -1;
  }
  http_run(argc, argv);
  void *status;
  pthread_join(tid, &status);
  return 0;
}
