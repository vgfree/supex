#include "router.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

char route_sign[7] = {0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a}; /* route\r\n*/

struct router_head *parse(char *data, uint32_t size)
{
  assert(data && size >= 17);
  struct router_head *head =
    (struct router_head*)malloc(sizeof(struct router_head));
  uint32_t nsize = 0;
  int i = 0;
  int fz = (data[i++] & 0x03) + 1;
  memcpy(&nsize, &data[i], fz);
  i = i + fz;

  assert(nsize >= 17);
  if (memcmp(route_sign, &data[i], 7) != 0) {
	printf("This is not router frame, error.");
	return NULL;
  }
  i += 7;
  head->message_from = data[i++];
  head->message_to = data[i++];
  memcpy(&head->CID_number, &data[i], 2);
  i += 2;
  head->cid = (struct CID*)calloc(head->CID_number, sizeof(struct CID));
  for (int j = 0; j < head->CID_number; j++) {
	memcpy(&head->cid[j].IP, &data[i], 4);
	i += 4;
    memcpy(&head->cid[j].fd, &data[i], 2);
	printf("fd:%d, data:%x,%x", head->cid[j].fd, data[i], data[i+1]);
	i += 2;
  }

  memcpy(&head->body_size, &data[i], 4);
  return head;
}

char *pack(char *data, uint32_t *size,
           const struct router_head *head)
{
  uint32_t headsz = 8 + (head->CID_number) * 6 + 8; // route\r\n + 2 + router_head
  int fsz = 0;
  if (headsz > (1 << 24) - 4) {
    fsz = 4;
  }
  else if (headsz > (1 << 16) -3) {
    fsz = 3;
  }
  else if (headsz > (1 << 8) -3) {
    fsz = 2;
  }
  else {
    fsz = 1;
  }
  *size = headsz + head->body_size +fsz;
  char *package = (char *)malloc((headsz + head->body_size + fsz) * sizeof(char));
  int i = 0;
  package[i++] = 0x04 + fsz -1; // FP_control, 
  if (fsz == 4) {
    package[i++] = headsz + fsz;
    package[i++] = (headsz + fsz) >> 8;
    package[i++] = (headsz + fsz) >> 16;
    package[i++] = (headsz + fsz) >> 24;
  }
  else if (fsz == 3) {
	package[i++] = headsz + fsz;
	package[i++] = (headsz + fsz) >> 8;
    package[i++] = (headsz + fsz) >> 16;
  }
  else if (fsz == 2) {
	package[i++] = headsz + fsz;
    package[i++] = (headsz + fsz) >> 8;
  }
  else {
    package[i++] = headsz + fsz;
  }

  memcpy(&package[i], route_sign, 7);
  i += 7;
  package[i++] = head->message_from;
  package[i++] = head->message_to;
  package[i++] = head->CID_number;
  package[i++] = head->CID_number >> 8;
  memcpy(&package[i], head->cid, head->CID_number * 6);
  i += head->CID_number * 6;
  package[i++] = head->body_size;
  package[i++] = head->body_size >> 8;
  package[i++] = head->body_size >> 16;
  package[i++] = head->body_size >> 24;

  memcpy(&package[i], data, head->body_size);
  return package;
}
