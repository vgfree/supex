#include "router.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

char route_sign[7] = {0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a}; /* route\r\n*/

struct router_head *parse_router(char *data, uint32_t size)
{
  if (data == NULL || size != 24) {
    printf("wrong data, size :%d", size);
    return NULL;
 }
  struct router_head *head =
    (struct router_head*)malloc(sizeof(struct router_head));
  if (memcmp(route_sign, data, 7) != 0) {
	printf("This is not router frame, error.");
	return NULL;
  }
  int i = 7;
  head->message_from = data[i++];
  head->message_to = data[i++];
  head->type = data[i++];
  switch (head->type) {
    case 0x0:
      memcpy(head->Cid.IP, &data[i], 4);
	  memcpy(&head->Cid.fd, &data[i+4], 2);
      break;
    case 0x01:
	  memcpy(head->Gid.gid, &data[i], 6);
      break;
    case 0x02:
	  memcpy(head->Uid.uid, &data[i], 6);
      break;
    case 0x03:
	  //reservev.
      break;
    case 0x04:
	  //reservev.
      break;
    default:
      break;
  }
  i += 6;
  i += 4; // reserved.
  memcpy(&head->body_size, &data[i], 4);
  return head;
}

char *pack_router(const struct router_head *head)
{
}
