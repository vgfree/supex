#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/alive_api.h"
#include "minor/sniff_api.h"
#include "alive_cpp_api.h"

#include "sniff_evcoro_lua_api.h"

#include "mttpsvp_libkv.h"
#include "mttpsvp_redis.h"

#define MTTPSVP_HEADER_SIZE             7
#define MAX_QUERY_STRING_PAIRES         64

extern struct sniff_cfg_list g_sniff_cfg_list;

typedef struct {
  const char*   base;
  size_t        len;
} qs_buf_t;

enum state {
  s_key_start = 0,
  s_key,
  s_value_start,
  s_value
};

static int parse_query_string(const char* data, size_t len, qs_buf_t *buf, size_t *buf_size) {
  char ch;
  const char *p;
  enum state s = s_key_start;
  size_t n = 0;
  int index = 0;

  const char *end = data + len;
  for (p = data; p < end; p++) {
    ch = *p;

    switch (s) {
      case s_key_start:
      {
        if (ch == '=' || ch == '&') {
          break;
        }

        buf[n].base = p;
        index++;
        s = s_key;
        break;
      }

      case s_key:
      {
        if (ch == '=') {
          buf[n].len = p - buf[n].base;
          index++;
          n++;
          s = s_value_start;
          break;
        }

        if (ch == '&') {
          buf[n].len = p - buf[n].base;
          n++;
          buf[n].base = NULL;
          buf[n].len = 0;

          n++;
          if (n >= MAX_QUERY_STRING_PAIRES * 2) {
            *buf_size = n / 2;
            return -1;
          }

          index = 0;
          s = s_key_start;
          break;
        }

        break;
      }

      case s_value_start:
      {
        if (ch == '=' || ch == '&') {
          buf[n].base = NULL;
          buf[n].len = 0;

          n++;
          if (n >= MAX_QUERY_STRING_PAIRES * 2) {
            *buf_size = n / 2;
            return -1;
          }

          index = 0;
          s = s_key_start;
          break;
        }

        buf[n].base = p;
        index++;
        s = s_value;
        break;
      }

      case s_value:
      {
        if (ch == '&') {
          buf[n].len = p - buf[n].base;
          n++;
          if (n >= MAX_QUERY_STRING_PAIRES * 2) {
            *buf_size = n / 2;
            return -1;
          }

          index = 0;
          s = s_key_start;
          break;
        }
        
        break;
      }
      
      default:
        return -1;
    }
  }

  switch (index) {
    case 0:
      break;
    case 1:
      buf[n].len = end - buf[n].base;
      n++;
      buf[n].base = NULL;
      buf[n].len = 0;
      break;
    case 2:
      buf[n].base = NULL;
      buf[n].len = 0;
      break;
    case 3:
      buf[n].len = end - buf[n].base;
      break;
    default:
      return -1;
  }

  n++;
  *buf_size = n / 2;
  return 0;
}

char *copy_query_string_value(qs_buf_t *buf, size_t buf_size, const char* key, size_t klen) {
  size_t i;
  for (i = 0; i < buf_size; i++) {
    size_t k = i * 2;
    size_t v = k + 1;
    if (buf[k].len == klen) {
      if (strncmp(buf[k].base, key, klen) == 0) {
        if (buf[v].base == NULL) {
          return NULL;
        } else {
          size_t len = buf[v].len;
          char *tmp = malloc(len + 1);
          if (tmp == NULL) return NULL;
          memcpy(tmp, buf[v].base, len);
          tmp[len] = '\0';
          return tmp;
        }
      }
    }
  }

  return NULL;
}

int alive_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	ALIVE_WORKER_PTHREAD    *p_alive_worker = (ALIVE_WORKER_PTHREAD *)user;
  char *errstr = NULL;
  size_t errlen = 0;

  size_t n = 0;
  char *mirrtalk_id;
  char *gps_token;
  qs_buf_t bufs[MAX_QUERY_STRING_PAIRES * 2];

  int size = task->size;
	if (size == 0) {
		// TODO
		// x_printf
    errstr = "data size is zero";
    errlen = strlen(errstr);
    goto ERROR;
	}

  size -= MTTPSVP_HEADER_SIZE;

	if (size > MAX_SNIFF_DATA_SIZE) {
		// TODO
		// x_printf
    errstr = "data size is greater than max sniff data size";
    errlen = strlen(errstr);
		goto ERROR;
	}

  uint8_t op_type = *((uint8_t*)task->data + 2);
  const char* data = (const char*)task->data + MTTPSVP_HEADER_SIZE;

  switch (op_type) {
    case 0x00:  // 握手
      x_printf(D, "handshake start");
      // 已经握手，重复握手，非正常数据，断开连接
      if (mttpsvp_libkv_check_handshake(task->cid, task->sfd, "1", 1) == 0) {
        errstr = "repeat handshake";
        errlen = strlen(errstr);
        goto ERROR;
      }

      int ret = parse_query_string(data, size, bufs, &n);
      if (ret < 0) {
        errstr = "parse_query_string error";
        errlen = strlen(errstr);
        goto ERROR;
      }
      x_printf(D, "%d", n);

      mirrtalk_id = copy_query_string_value(bufs, n, "M", 1);
      if (mirrtalk_id == NULL) {
        errstr = "can't get mirrtalk_id M=XXXXXX";
        errlen = strlen(errstr);
        goto ERROR;
      }

      gps_token = copy_query_string_value(bufs, n, "N", 1);
      if (gps_token == NULL) {
        free(mirrtalk_id);
        errstr = "can't get gps_token N=XXXXXX";
        errlen = strlen(errstr);
        goto ERROR;
      }

      if (mttpsvp_redis_check_gpstoken(mirrtalk_id, gps_token, strlen(gps_token)) < 0) {
        errstr = "gpsToken error";
        errlen = strlen(errstr);
        free(mirrtalk_id);
        free(gps_token);
        goto ERROR;
      }

      /*gpsToken is used only once*/
      mttpsvp_redis_del_gpstoken(mirrtalk_id);

      mttpsvp_libkv_set(task->cid, task->sfd, "1");
      free(mirrtalk_id);
      free(gps_token);
      x_printf(D, "handshake end");
      return 0;
    case 0x01:  // 心跳
      x_printf(D, "heart start");
      return 0;
    case 0x02:  // 传输
      // 未握手，直接传数据，非正常数据，断开连接
      x_printf(D, "transfer start");
      if (mttpsvp_libkv_check_handshake(task->cid, task->sfd, "1", 1) < 0) {
        errstr = "shoule handshake first before transfer data";
        errlen = strlen(errstr);
        goto ERROR;
      }

      break;
    default:
      errstr = "unknow long connection type";
      errlen = strlen(errstr);
      goto ERROR;
  }

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;
	sniff_task.last = false;
	sniff_task.stamp = time(NULL);
	sniff_task.size = size;
	memcpy(sniff_task.data, data, size);

	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)p_alive_worker->mount;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_alive_worker->mount = p_sniff_worker->next;
  x_printf(D, "transfer start");
	return 0;

ERROR:
  x_printf(D, "%s", errstr);
  mttpsvp_libkv_del(task->cid, task->sfd);
  /*alive_send_data(1, task->cid, task->sfd, errstr, errlen);*/
  alive_close_conn(1, task->cid, task->sfd);
  return -1;
}

int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task) {
  x_printf(D, "online");
  mttpsvp_libkv_set(task->cid, task->sfd, "0");
  return 0;
}

int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task) {
  x_printf(D, "offline");
  mttpsvp_libkv_del(task->cid, task->sfd);
  return 0;
}
