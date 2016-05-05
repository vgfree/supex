#include "loger.h"
#include "gid_map.h"

#include <string.h>

static kv_handler_t *g_gid_map = NULL;

void init_gid_map()
{
  g_gid_map = kv_create(NULL);
}

int find_fd_list(char *gid, int fd_list[])
{
  char cmd[30] = "lrange ";
  strcat(cmd, gid);
  strcat(cmd, " 0 -1");
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    error("find multi fd error, cmd:%s", cmd);
    return -1;
  }
  int i = 0;
  kv_answer_value_t *value;
  kv_answer_iter_t *iter;

  iter = kv_answer_get_iter(ans, ANSWER_HEAD);
  kv_answer_rewind_iter(ans, iter);
  while ((value = kv_answer_next(iter)) != NULL) {
    fd_list[i] = ((char*)value->ptr)[1];
    fd_list[i] = fd_list[i] * 256 + ((char*)value->ptr)[0];
	i++;
  }
  kv_answer_release_iter(iter);
  return i;
}

int insert_fd_list(char *gid, int fd_list[], int size)
{
  char cmd[GROUP_SIZE * 2 + 20] = "lpush ";
  strcat(cmd, gid);
  for (int i = 0; i < size; i++) {
    char buf[4];
    buf[0] = 32;
    buf[1] = fd_list[i] % 256;
    buf[2] = fd_list[i] / 256;
    buf[3] = '\0';
    strcat(cmd, buf);
  }
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    error("errnum:%d\terr:%s", ans->errnum, ans->err);
    return -1;
  }
  return 0;
}

int remove_fd_list(char *gid, int fd_list[], int size)
{
  char cmd[GROUP_SIZE * 2 + 20] = "lrem ";
  strcat(cmd, gid);
  for (int i = 0; i < size; i++) {
    char buf[4];
    buf[0] = 32;
    buf[1] = fd_list[i] % 256;
    buf[2] = fd_list[i] / 256;
    buf[3] = '\0';
    strcat(cmd, buf);
  }
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    error("errnum:%d\terr:%s", ans->errnum, ans->err);
    return -1;
  }
  return 0;
}

void destroy_gid_map()
{
  kv_destroy(g_gid_map);
}
