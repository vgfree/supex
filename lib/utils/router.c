#include "communication.h"
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
  memcpy(&head->identity, data, 6);
  i += 6;
  memcpy(&head->body_size, &data[i], 4);
  return head;
}

struct comm_message *pack_router(const struct router_head *head,
                                 const struct comm_message *msg)
{
  assert(head && msg);
  struct comm_message *message = (struct comm_message *)malloc(sizeof(struct comm_message));
  message->fd = msg->fd;
  uint32_t rou_Frame_Size = 7 + 3 + 6 + 4 + head->body_size;
  message->dsize = msg->dsize + rou_Frame_Size;
  message->frames = msg->frames + 1;
  message->packages = msg->packages;
  message->frame_offset[0] = 0;
  for (int i = 0; i < msg->frames; i++) {
    message->frame_offset[i+1] = msg->frame_offset[i] + rou_Frame_Size;
  }
  for (int i = 0; i < msg->packages; i++) {
    message->frames_of_package[i] = msg->frames_of_package[i];
  }
  message->encryption = msg->encryption;
  message->compression = msg->compression;
  message->socket_type = msg->socket_type;
  message->content = (char *)malloc(message->dsize * sizeof(char));
  memcpy(message->content, route_sign, 7);
  message->content[8] = head->message_from;
  message->content[9] = head->message_to;
  message->content[10] = head->type;
  memcpy(message->content + 11, &head->identity, 6);
  memcpy(message->content + 17, &head->body_size, 4);
  memcpy(message->content + 21, head->body, head->body_size);
  memcpy(message->content + rou_Frame_Size, msg->content, msg->dsize);
  return message;
}
