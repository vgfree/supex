#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/alive_api.h"
#include "minor/sniff_api.h"
#include "alive_cpp_api.h"

#include "sniff_evcoro_lua_api.h"

#include "mttpsvp_libkv.h"
#include "mttpsvp_redis.h"

#define MTTPSVP_HEADER_SIZE     7
#define MAX_QUERY_STRING_PAIRES 64
#define ERROR_STRING_LEN        256
#define ERROR_STRING_SIZE       sizeof("&TYPE=0&ERROR=0") - 1

extern struct sniff_cfg_list g_sniff_cfg_list;

typedef struct
{
	const char      *base;
	size_t          len;
} qs_buf_t;

enum state
{
	s_key_start = 0,
	s_key,
	s_value_start,
	s_value
};

static int parse_query_string(const char *data, size_t len, qs_buf_t *buf, size_t *buf_size)
{
	char            ch;
	const char      *p;
	enum state      s = s_key_start;
	size_t          n = 0;
	int             index = 0;

	const char *end = data + len;

	for (p = data; p < end; p++) {
		ch = *p;

		switch (s)
		{
			case s_key_start:
			{
				if ((ch == '=') || (ch == '&')) {
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
				if ((ch == '=') || (ch == '&')) {
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

	switch (index)
	{
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

char *copy_query_string_value(qs_buf_t *buf, size_t buf_size, const char *key, size_t klen)
{
	size_t i;

	for (i = 0; i < buf_size; i++) {
		size_t  k = i * 2;
		size_t  v = k + 1;

		if (buf[k].len == klen) {
			if (strncmp(buf[k].base, key, klen) == 0) {
				if (buf[v].base == NULL) {
					return NULL;
				} else {
					size_t  len = buf[v].len;
					char    *tmp = malloc(len + 1);

					if (tmp == NULL) {
						return NULL;
					}

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
	ALIVE_WORKER_PTHREAD *p_alive_worker = (ALIVE_WORKER_PTHREAD *)user;

	size_t                  n = 0;
	char                    *mirrtalk_id;
	char                    *gps_token;
	qs_buf_t                bufs[MAX_QUERY_STRING_PAIRES * 2];
	char                    err[ERROR_STRING_LEN];
	struct sniff_task_node  sniff_task = {};

	int     size = task->size;
	int     plus_size = 0;

	if (size == 0) {
		x_printf(D, "Error: data size is zero");
		goto ERROR;
	}

	size -= MTTPSVP_HEADER_SIZE;

	/*for test*/
	if (size > MAX_SNIFF_DATA_SIZE - 64) {
		size = 0;
		x_printf(D, "Error: data size(%d) is greater than MAX_SNIFF_DATA_SIZE(%d)", size, MAX_SNIFF_DATA_SIZE);
		goto ERROR;
	}

	struct pool_node *pnode = mapping_pool_addr(task->sfd);

	if (pnode->cid != task->cid) {
		x_printf(D, "Error: pool_node->cid(%d) and task->cid(%d) not equal", pnode->cid, task->cid);
		goto ERROR;
	}

	uint8_t         op_type = *((uint8_t *)task->data + 2);
	const char      *data = (const char *)task->data + MTTPSVP_HEADER_SIZE;

	switch (op_type)
	{
		case 0x00:	// 握手

			// 已经握手，重复握手，非正常数据，断开连接
			if (mttpsvp_libkv_check_handshake(task->cid, task->sfd, "1", 1) == 0) {
				x_printf(D, "Handshake error: repeat handshake");
				x_printf(D, "Handshake data: %s", data);
				goto ERROR;
			}

			int ret = parse_query_string(data, size, bufs, &n);

			if (ret < 0) {
				x_printf(D, "Handshake error: parse data error");
				x_printf(D, "Handshake data: %s", data);
				goto ERROR;
			}

			mirrtalk_id = copy_query_string_value(bufs, n, "M", 1);

			if (mirrtalk_id == NULL) {
				x_printf(D, "Handshake error: can't get mirrtalkID");
				x_printf(D, "Handshake data: %s", data);
				goto ERROR;
			}

			gps_token = copy_query_string_value(bufs, n, "N", 1);

			if (gps_token == NULL) {
				free(mirrtalk_id);
				x_printf(D, "Handshake error: can't get gpsToken");
				x_printf(D, "Handshake data: %s", data);
				goto ERROR;
			}

			if (mttpsvp_redis_check_gpstoken(mirrtalk_id, gps_token, strlen(gps_token)) < 0) {
				x_printf(D, "Handshake error: gpsToken error");
				x_printf(D, "Handshake data: %s", data);
				free(mirrtalk_id);
				free(gps_token);
				goto ERROR;
			}

			/*gpsToken is used only once*/
			mttpsvp_redis_del_gpstoken(mirrtalk_id);

			mttpsvp_libkv_set(task->cid, task->sfd, "1");

			x_printf(D, "Handshake ok, mirrtalkID: %s, ip: %s, port: %d", mirrtalk_id, pnode->szAddr, pnode->port);
			x_printf(D, "Handshake data: %s", data);
			plus_size = sprintf(err, "&TYPE=%d&IP=%s&PORT=%d", 0, pnode->szAddr, pnode->port);

			free(mirrtalk_id);
			free(gps_token);
			/*return 0;*/
			break;

		case 0x01:	// 心跳
			x_printf(D, "Heartbeat");
			plus_size = sprintf(err, "TYPE=%d&IP=%s&PORT=%d", 1, pnode->szAddr, pnode->port);
			/*return 0;*/
			break;

		case 0x02:	// 传输

			// 未握手，直接传数据，非正常数据，断开连接
			if (mttpsvp_libkv_check_handshake(task->cid, task->sfd, "1", 1) < 0) {
				x_printf(D, "Should handshake first before transfer data");
				goto ERROR;
			}

			plus_size = sprintf(err, "&TYPE=%d&IP=%s&PORT=%d", 2, pnode->szAddr, pnode->port);
			break;

		default:
			x_printf(D, "Unknow connection type");
			goto ERROR;
	}

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;
	sniff_task.last = false;
	sniff_task.stamp = time(NULL);
	sniff_task.size = size + plus_size;
	memcpy(sniff_task.data, data, size);
	memcpy((char *)sniff_task.data + size, err, plus_size);

	g_sniff_cfg_list.task_report(p_alive_worker->mount, &sniff_task);
	return 0;

// start
    key = // 从客户端接收的数据提取key.
	fd = hkey_get_fd(key); // 通过接收到的key 寻找cid.
	if (fd != -1) // 如该cid 存在， 那么可能之前有数据存储。
	{
		struct residue_package package;
		char *value = hkey_get_value(key); // 从对应的libkv 中取得之前存的package.
		int offset = hkey_get_offset(key); //数据的偏移.
		init_residue_package(&package, task->cid.fd, offset, value, strlen(value)); // 不支持二进制。
		push_residue_package(&package);// 把package 重新存到io thread 的buffer.
		hkey_del_fd(key); // 把之前存储的所有对应内容清空。
		hkey_del_value(key);
		hkey_del_fd_key(fd);
		destroy_residue_package(&package);
	}
	else {
		// 如果不存在， 那么是第一次握手，不用处理。
	}
	hkey_insert_fd(key, cid.fd); // 存key, cid. 更新对应的key 与cid 的bind.
// end

ERROR:
	mttpsvp_libkv_del(task->cid, task->sfd);
	alive_close_conn(1, task->cid, task->sfd);
	return 0;
}

int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	// start
	// 不用处理。
	// end
	mttpsvp_libkv_set(task->cid, task->sfd, "0");
	return 0;
}

int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	mttpsvp_libkv_del(task->cid, task->sfd);
	//start
	struct residue_package package; // 用于提取来自io thread 中的buffer.
	pop_residue_pakage(&package);  // 从io thread 提取。
	char *key = hkey_find_key(task->cid.fd); // 从cid 中找对应的key.
	hkey_insert_value(key, &package);  // 把key 对应的package 存储起来.
	//end
	return 0;
}

