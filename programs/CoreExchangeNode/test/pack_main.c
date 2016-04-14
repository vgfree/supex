#include "../router.h"

#include <string.h>
#include <stdio.h>

char expect_value[25] = {0x04, 0x17, 0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a,
  0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02
};

int test_parse_function() {
  struct router_head exp_val;
  exp_val.message_from = 0x01;
  exp_val.message_to = 0x00;
  exp_val.CID_number = 0x01;
  exp_val.cid = (struct CID*)malloc(sizeof(struct CID));
  exp_val.cid[0].IP = 0x00;
  exp_val.cid[0].fd = 0x07;
  exp_val.body_size = 0x02;
  struct router_head *test_head =
    parse(expect_value, 25);
  printf("\nexp : from:%d, to:%d, number:%d, ip:%d, fd:%d, size:%d.\n",
         exp_val.message_from, exp_val.message_to, exp_val.CID_number,
         exp_val.cid[0].IP, exp_val.cid[0].fd, exp_val.body_size);
  printf("test: from:%d, to:%d, number:%d, ip:%d, fd:%d, size:%d.\n",
         test_head->message_from, test_head->message_to, test_head->CID_number,
         test_head->cid[0].IP, test_head->cid[0].fd, test_head->body_size);
  if ((exp_val.message_from == test_head->message_from) &&
      (exp_val.message_to == test_head->message_to) &&
      (exp_val.CID_number == test_head->CID_number) &&
	  (exp_val.cid[0].IP == test_head->cid[0].IP) &&
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

int test_pack_function() {
  char data[2] = {0x00, 0x02}; //空包。
  struct router_head head;
  head.message_from = 0x01;
  head.message_to = 0x00;
  head.CID_number = 0x01;
  head.cid = (struct CID*)malloc(sizeof(struct CID));
  head.cid[0].IP = 0x00;
  head.cid[0].fd = 0x07;
  head.body_size = 0x02;
  uint32_t size = 0;
  char *package = pack(data, &size, &head);
  printf("-------------------tested package function--------------\n package size:%d.------------\n", size);
  int exp_value = 0;
  for (int i = 0; i < size; i++) {
	  printf("0x%x-0x%x ", expect_value[i], package[i]);
	  if (expect_value[i] != package[i]) {
		  exp_value = -1;
		  break;
	  }
  }
  return exp_value;
}
int main(int argc, char *argv[])
{
  if (test_pack_function() == 0) {
    printf("\n ---------pack fucntion OK-----------\n");
  }
  else {
	printf("\n -----------parse error----------\n");
  }
  printf("\n");
  if (test_parse_function() == 0) {
    printf("\n -------parse function OK------------\n");	  
  }
  else {
    printf("\n -----parse error -----\n");
  }
  return 0;
}
