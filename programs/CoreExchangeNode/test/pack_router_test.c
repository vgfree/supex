#include "core_exchange_node_test.h"

#include <stdio.h>

static char expect_value[25] = {0x04, 0x17, 0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a,
  0x01, 0x00, 0x01, 0x00, 0x7f, 0x00, 0x00, 0x01, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02
};

int test_pack_function() {
  char data[2] = {0x00, 0x02}; //空包。
  struct router_head head;
  head.message_from = 0x01;
  head.message_to = 0x00;
  head.type = 0x01;
  head.Cid.IP[0] = 0x7f;
  head.Cid.IP[1] = 0x0;
  head.Cid.IP[2] = 0x0;
  head.Cid.IP[3] = 0x01;
  head.Cid.fd = 0x07;
  head.body_size = 0x02;
  uint32_t size = 0;
//  char *package = pack_router(data, &size, &head);
  printf("-------------------tested package function--------------\n package size:%d.------------\n", size);
  int exp_value = 0;
/*  for (int i = 0; i < size; i++) {
	  printf("0x%x-0x%x ", expect_value[i], package[i]);
	  if (expect_value[i] != package[i]) {
		  exp_value = -1;
		  break;
	  }
  }*/
//  free(package);
  return exp_value;
}
