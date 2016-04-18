#include "core_exchange_node_test.h"

#include <stdio.h>

static char expect_value[25] = {0x04, 0x17, 0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a,
  0x01, 0x00, 0x01, 0x00, 0x7f, 0x00, 0x00, 0x01, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02
};

int test_parse_function() {
  struct router_head exp_val;
  exp_val.message_from = 0x01;
  exp_val.message_to = 0x00;
  exp_val.CID_number = 0x01;
  exp_val.cid = (struct CID*)malloc(sizeof(struct CID));
  exp_val.cid[0].IP[0] = 0x7f;
  exp_val.cid[0].IP[1] = 0x0;
  exp_val.cid[0].IP[2] = 0x0;
  exp_val.cid[0].IP[3] = 0x01;
  exp_val.cid[0].fd = 0x07;
  exp_val.body_size = 0x02;
  struct router_head *test_head =
    parse_router(expect_value, 25);
  printf("\nexp : from:%d, to:%d, number:%d, ip[0]:%x, fd:%d, size:%d.\n",
         exp_val.message_from, exp_val.message_to, exp_val.CID_number,
         exp_val.cid[0].IP[0], exp_val.cid[0].fd, exp_val.body_size);
  printf("test: from:%d, to:%d, number:%d, ip[0]:%d, fd:%d, size:%d.\n",
         test_head->message_from, test_head->message_to, test_head->CID_number,
         test_head->cid[0].IP[0], test_head->cid[0].fd, test_head->body_size);
  if ((exp_val.message_from == test_head->message_from) &&
      (exp_val.message_to == test_head->message_to) &&
      (exp_val.CID_number == test_head->CID_number) &&
	  (exp_val.cid[0].IP[0] == test_head->cid[0].IP[0]) &&
	  (exp_val.cid[0].IP[1] == test_head->cid[0].IP[1]) &&
	  (exp_val.cid[0].IP[2] == test_head->cid[0].IP[2]) &&
	  (exp_val.cid[0].IP[3] == test_head->cid[0].IP[3]) &&
	  (exp_val.cid[0].fd == test_head->cid[0].fd) &&
	  (exp_val.body_size == test_head->body_size))  {
    free(exp_val.cid);
    free(test_head->cid);
	free(test_head);
    return 0;
  }
  free(exp_val.cid);
  free(test_head->cid);
  free(test_head);
  return -1;
}
