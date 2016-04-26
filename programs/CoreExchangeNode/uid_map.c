#include "loger.h"
#include "uid_map.h"

#include <string.h>

void init_uid_map()
{
  g_uid_map = kv_create(NULL);
}

int find_fd(char *uid)
{
  char cmd[20] = "get ";
  strcat(cmd, uid);
  kv_answer_t *ans;
  ans = kv_ask(g_uid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    log("errnum:%d\terr:%s", ans->errnum, ans->err);
    return -1;
  }

  kv_answer_value_t *value = kv_answer_first_value(ans);
  if (value->ptrlen != 2) {
    error("value length not equal 2.");
    return -1;
  }
  int fd = ((char*)value->ptr)[1];
  fd = fd * 256 + ((char*)value->ptr)[0];
  return fd;
}

int insert_fd(char *uid, int fd)
{
  char cmd[30] = "set ";
  strcat(cmd, uid);
  char buf[4];
  buf[0] = 32;
  buf[1] = fd % 256;
  buf[2] = fd / 256;
  buf[3] = '\0';
  strcat(cmd, buf);
  kv_answer_t *ans = kv_ask(g_uid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    log("error.");
    return -1;
  }
  return 0;
}

int remove_fd(char *uid)
{
  char cmd[20] = "del ";
  strcat(cmd, uid);
  kv_answer_t *ans = kv_ask(g_uid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    error("removed error.");
    return -1;
  }
  return 0;
}

void destroy_uid_map()
{
  kv_destroy(g_uid_map);
}
