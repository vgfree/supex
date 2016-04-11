#include "fd_manager.h"
#include "loger.h"

#include <assert.h>

#define FD_CAPACITY 10000

static struct fd_list g_list;
static struct fd_array g_array;

int list_init()
{
  for (int i = 0; i < FD_MAX_CLASSIFICATION; i++) {
    g_list.head[i].size = 0;
    g_list.head[i].next = NULL;
  }
  return SUCCESS;
}

int list_destroy()
{
  for (int i = 0; i < FD_MAX_CLASSIFICATION; i++) {
    struct fd_node *curr;
    struct fd_node *next;
    curr = g_list.head[i].next;
    while (curr){
      next = curr->next;
      free(curr);
      curr = next;
    }
    g_list.head[i].next = NULL;
  }
  return SUCCESS;
}

int list_remove(const enum router_object obj,
                const int fd)
{
  struct fd_node **curr = &g_list.head[obj].next;
  while (*curr) {
    struct fd_node *entry = *curr;
    if (entry->fd == fd) {
      *curr = entry->next;
      free(entry);
      return SUCCESS;
    }
    else {
      curr = &entry->next;
    }
  }
  return FAILED;
}

int list_push_back(const enum router_object obj,
                   const struct fd_node *node)
{
  struct fd_node *curr = g_list.head[obj].next;
  while (curr->next) {
    curr = curr->next;
  }
  curr->next = (struct fd_node *)malloc(sizeof(struct fd_node));
  curr->next->fd = node->fd;
  curr->next->status = node->fd;
  curr->next->next = NULL;
  return SUCCESS;
}

int list_front(const enum router_object obj,
               struct fd_node *node)
{
  if (g_list.head[obj].next) {
    *node = *g_list.head[obj].next;
    return SUCCESS;
  }
  return FAILED;
}

int array_init()
{
  g_array.dsp_array = (struct fd_descriptor *)
    calloc(FD_CAPACITY, sizeof(struct fd_descriptor));
  assert(g_array.dsp_array);
  g_array.max_fd = 0;
  g_array.cap = FD_CAPACITY;

  return SUCCESS;
}

int array_destroy()
{
  if (g_array.dsp_array == NULL) {
    return FAILED;
  }
  free(g_array.dsp_array);
  g_array.max_fd = 0;
  g_array.cap = 0;
  return FAILED;
}

int array_fill_fd(const int fd, const struct fd_descriptor *des)
{
  if (fd > g_array.cap) {
    error("fd array not enough capacity.");
    // to do.
  }
  assert(g_array.dsp_array);

  g_array.dsp_array[fd] = *des;
  if (g_array.max_fd < fd) {
    g_array.max_fd = fd;
  }
  else {
    warn("max_fd >fd ?");
  }
  return SUCCESS;
}

int array_remove_fd(const int fd)
{
  g_array.dsp_array[fd].status = 2;
  return FAILED;
}

int array_at_fd(const int fd, struct fd_descriptor *des)
{
  des->ip = g_array.dsp_array[fd].ip;
  des->port = g_array.dsp_array[fd].port;
  des->status = g_array.dsp_array[fd].status;
  des->obj = g_array.dsp_array[fd].obj;

  return SUCCESS;
}
