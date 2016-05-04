#include "http_dispatch.h"
#include "message_concentrator.h"

int main(int argc, char *argv[])
{
  if (concentrator_init() != 0) {
    printf("concentrator not init.");
	return -1;
  }
  http_run(argc, argv);
  return 0;
}
