//#include "loger.h"
#include "gid_map.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static kv_handler_t *g_gid_map = NULL;

void init_gid_map()
{
  g_gid_map = kv_create(NULL);
}

int find_fd_list(char *gid, int *fd_list)
{
  char cmd[30] = "lrange ";
  strcat(cmd, gid);
  strcat(cmd, " 0 -1");
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    printf("find multi fd error, cmd:%s\n", cmd);
    return -1;
  }
  int i = 0;
  kv_answer_value_t *value;
  kv_answer_iter_t *iter;

  iter = kv_answer_get_iter(ans, ANSWER_HEAD);
  kv_answer_rewind_iter(ans, iter);
  while ((value = kv_answer_next(iter)) != NULL) {
    char buf[20] = {};
    strncpy(buf, (char*)value->ptr, value->ptrlen);
    printf("find fd:%s.\n", buf);
    fd_list[i] = atoi(buf);
	i++;
  }
  kv_answer_release_iter(iter);
  return i;
}

int insert_fd_list(char *gid, int fd_list[], int size)
{
  char cmd[GROUP_SIZE * 10 + 20] = "lpush ";
  strcat(cmd, gid);
  for (int i = 0; i < size; i++) {
    strcat(cmd, " ");
    char buf[10];
	snprintf(buf, 10, "%d", fd_list[i]);
    strcat(cmd, buf);
  }
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    printf("errnum:%d\terr:%s\n", ans->errnum, ans->err);
    return -1;
  }
  return 0;
}

int remove_fd_list(char *gid, int fd_list[], int size)
{
  for (int i = 0; i < size; i++) {
    char cmd[50] = "lrem ";
    strcat(cmd, gid);
    strcat(cmd, " 0 ");
    char buf[10];
	snprintf(buf, 10, "%d", fd_list[i]);
    strcat(cmd, buf);
    printf("remove_fd_list, :%s\n", cmd);
    kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
    if (ans->errnum != ERR_NONE) {
      printf("errnum:%d\terr:%s\n", ans->errnum, ans->err);
      return -1;
    }
  }
  return 0;
}

int insert_gid_list(int fd, char *gid)
{
  char cmd[20 + 20] = "lpush ";
  char buf[10] = {};
  snprintf(buf, 10, "%d", fd);
  strcat(cmd, buf);
  strcat(cmd, " ");
  strcat(cmd, gid);
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    printf("errnum:%d\terr:%s\n", ans->errnum, ans->err);
    return -1;
  }
  return 0;
}

int find_gid_list(int fd, char *gid_list[], int *size)
{
  
  printf("fd:%d.\n", fd);
  char cmd[30] = "lrange ";
  char buf[10] = {};
  snprintf(buf, 10, "%d", fd);
  printf("fd:%d.\n", fd);
  printf("buf:%s\n", buf);
  strcat(cmd, buf);
  printf("cmd:%s\n", cmd);
  strcat(cmd, " 0 -1");
  printf("cmd:%s\n", cmd);
  kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
  if (ans->errnum != ERR_NONE) {
    printf("find multi fd error, cmd:%s\n", cmd);
    return -1;
  }
  int gid_count = 0;
  kv_answer_value_t *value;
  kv_answer_iter_t *iter;

  iter = kv_answer_get_iter(ans, ANSWER_HEAD);
  kv_answer_rewind_iter(ans, iter);
  while ((value = kv_answer_next(iter)) != NULL) {
    printf("value->ptr:%s, value->ptrlen:%d.\n", (char *)value->ptr, value->ptrlen);
    gid_list[gid_count] = (char *)malloc(value->ptrlen * sizeof(char) + 1);
	strncpy(gid_list[gid_count], (char*)value->ptr, value->ptrlen);
	gid_count++;
  }
  kv_answer_release_iter(iter);
  *size = gid_count;
  return gid_count;
}

int remove_gid_list(int fd, char *gid[], int size)
{
  char buf[10] = {};
  snprintf(buf, 10, "%d", fd);
  for (int i = 0; i < size; i++) {
    char cmd[50] = "lrem ";
    strcat(cmd, buf);
    strcat(cmd, " 0 ");
    strcat(cmd, gid[i]);
    printf("remove_gid_list:%s\n", cmd);
    kv_answer_t *ans = kv_ask(g_gid_map, cmd, strlen(cmd));
    if (ans->errnum != ERR_NONE) {
      printf("errnum:%d\terr:%s\n", ans->errnum, ans->err);
      return -1;
    }
  }
  return 0;
}

void destroy_gid_map()
{
  kv_destroy(g_gid_map);
}
