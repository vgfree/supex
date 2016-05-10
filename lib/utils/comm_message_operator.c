#include "comm_message_operator.h"
#include "loger.h"

#include <assert.h>

int get_msg_fd(struct comm_message *msg)
{
  assert(msg);
  return msg->fd;
}

void set_msg_fd(struct comm_message *msg, int fd)
{
  assert(msg);
  msg->fd = fd;
}

char *get_msg_frame(int index, struct comm_message *msg, int *size)
{
  assert(msg && msg->content);
  if (index >= msg->package.frames || index < 0) {
    error("index:%d > max frames:%d.", index, msg->package.frames);
    return NULL;
  }
  *size = msg->package.frame_size[index];
  return msg->content + msg->package.frame_offset[index];
}

int set_msg_frame(int index, struct comm_message *msg, int size, char *frame)
{
  // 默认msg->content 已经malloc 了足够大的空间。
  assert(msg && msg->content && frame);
  if (index > msg->package.frames || index < 0) {
    error("index:%d > max frames:%d.", index, msg->package.frames);
    return -1;
  }
  if (index == msg->package.frames) {
    memcpy(msg->content + msg->package.dsize, frame, size);
    msg->package.frames ++;
    msg->package.frame_size[index] = size;
    msg->package.frame_offset[index] = msg->package.dsize;
    msg->package.dsize += size;
  }
  else {
    memmove(msg->content + msg->package.frame_offset[index] + size,
            msg->content + msg->package.frame_offset[index],
            msg->package.dsize - msg->package.frame_offset[index]); 
    memcpy(msg->content + msg->package.frame_offset[index],
           frame, size);
    for (int i = index; i < msg->package.frames; i++) {
      msg->package.frame_size[i + 1] = msg->package.frame_size[i];
      msg->package.frame_offset[i + 1] = msg->package.frame_offset[i] + size;
    }
    msg->package.frames ++;
    msg->package.frame_size[index] = size;
    msg->package.dsize += size;
  }
  return 0;
}
