#include "comm_message_operator.h"
#include "fd_manager.h"
#include "loger.h"
#include "message_dispatch.h"
#include "notify.h"
#include "status.h"

void client_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr)
{
	assert(g_serv_info.commctx == commctx);
	log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
	switch (portinfo->stat)
	{
		case FD_INIT:
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = CLIENT;
			array_fill_fd(portinfo->fd, &des);
			send_status_msg(portinfo->fd, FD_INIT);
		}
		break;

		case FD_CLOSE:
		{
			array_remove_fd(portinfo->fd);
			erase_client(portinfo->fd);
			send_status_msg(portinfo->fd, FD_CLOSE);
			log("errase client fd:%d.", portinfo->fd);
#ifdef _HKEY_
			struct residue_package package;
			init_residue_package(&package, portinfo->fd, 0, NULL, 0);
			pop_residue_package(&package);
			char *key = hkey_find_key(portinfo->fd);
			hkey_insert_value(key, package.serial_data);
			hkey_insert_offset(key, package.offset);
			destroy_residue_package(&package);
			free(key);
#endif
		}
		break;

		default:
			break;
	}
}

void message_gateway_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr)
{
	if (g_serv_info.commctx != commctx) {
		error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
			g_serv_info.commctx, commctx);
	}

	log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = MESSAGE_GATEWAY;
			array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			g_serv_info.message_gateway_fd = portinfo->fd;
			node.status = 1;
			list_push_back(MESSAGE_GATEWAY, &node);
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			array_at_fd(portinfo->fd, &des);
			log("array_at_fd, status:%d, obj:%d.", des.status, des.obj);

			if (des.status != 1) {
				error("this fd:%d is not running.", portinfo->fd);
				return;
			}

			array_remove_fd(portinfo->fd);

			if (g_serv_info.message_gateway_fd == portinfo->fd) {
				g_serv_info.message_gateway_fd = 0;
				list_remove(MESSAGE_GATEWAY, portinfo->fd);
			} else {
				error("this fd:%d is not belong to MESSAGE_GATEWAY.", portinfo->fd);
			}

			break;
		}

		default:
			break;
	}
}

void setting_server_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr)
{
	if (g_serv_info.commctx != commctx) {
		error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
			g_serv_info.commctx, commctx);
	}

	log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = SETTING_SERVER;
			array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			g_serv_info.setting_server_fd = portinfo->fd;
			node.status = 1;
			list_push_back(SETTING_SERVER, &node);
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			array_at_fd(portinfo->fd, &des);
			log("array_at_fd, status:%d, obj:%d.", des.status, des.obj);

			if (des.status != 1) {
				error("this fd:%d is not running.", portinfo->fd);
				return;
			}

			array_remove_fd(portinfo->fd);

			if (g_serv_info.setting_server_fd == portinfo->fd) {
				error("setting_server failed.");
				g_serv_info.setting_server_fd = 0;
				list_remove(SETTING_SERVER, portinfo->fd);
			} else {
				error("this fd:%d is not belong to SETTING_SERVER.", portinfo->fd);
			}

			break;
		}

		default:
			break;
	}
}

void login_server_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr)
{
	if (g_serv_info.commctx != commctx) {
		error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
			g_serv_info.commctx, commctx);
	}

	log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = LOGIN_SERVER;
			array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			g_serv_info.login_server_fd = portinfo->fd;
			node.status = 1;
			list_push_back(LOGIN_SERVER, &node);
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			array_at_fd(portinfo->fd, &des);
			log("array_at_fd, status:%d, obj:%d.", des.status, des.obj);

			if (des.status != 1) {
				error("this fd:%d is not running.", portinfo->fd);
				return;
			}

			array_remove_fd(portinfo->fd);

			if (g_serv_info.login_server_fd == portinfo->fd) {
				g_serv_info.login_server_fd = 0;
				list_remove(LOGIN_SERVER, portinfo->fd);
			} else {
				error("this fd:%d is not belong to LOGIN_SERVER.", portinfo->fd);
			}

			break;
		}

		default:
			break;
	}
}

