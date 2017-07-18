#include <uuid/uuid.h>

#include "comm_message_operator.h"
#include "fd_manager.h"
#include "message_dispatch.h"
#include "notify.h"
#include "status.h"
#include "libmini.h"

void make_uuid(char *dst)
{
	uuid_t  uuid;
	uuid_generate_time(uuid);
	uuid_unparse(uuid, dst);
}

void client_event_notify(struct comm_context *commctx,
	struct comm_tcp *portinfo, void *usr)
{
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
	switch (portinfo->stat)
	{
		case FD_INIT:
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = CLIENT;
			make_uuid(des.uuid);
			fdman_array_fill_fd(portinfo->fd, &des);
			
			send_status_msg(portinfo->fd, FD_INIT);
		}
		break;

		case FD_CLOSE:
		{
			struct fd_descriptor des = {};
			fdman_array_remove_fd(portinfo->fd, &des);
			
			erase_client(portinfo->fd);
			x_printf(D, "errase client fd:%d.", portinfo->fd);
			
			send_status_msg(portinfo->fd, FD_CLOSE);

			commapi_close(commctx, portinfo->fd);
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
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);

	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = MESSAGE_GATEWAY;
			make_uuid(des.uuid);
			fdman_array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			node.status = 1;
			fdman_list_push_back(MESSAGE_GATEWAY, &node);
			
			g_serv_info.message_gateway_fd = portinfo->fd;
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			fdman_array_remove_fd(portinfo->fd, &des);
			assert(des.status == 1);

			if (g_serv_info.message_gateway_fd == portinfo->fd) {
				x_printf(E, "message_server failed.");
				g_serv_info.message_gateway_fd = 0;
				fdman_list_remove(MESSAGE_GATEWAY, portinfo->fd);
			} else {
				x_printf(S, "this fd:%d is not belong to MESSAGE_GATEWAY.", portinfo->fd);
			}

			commapi_close(commctx, portinfo->fd);
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
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);

	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = SETTING_SERVER;
			make_uuid(des.uuid);
			fdman_array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			node.status = 1;
			fdman_list_push_back(SETTING_SERVER, &node);
			
			g_serv_info.setting_server_fd = portinfo->fd;
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			fdman_array_remove_fd(portinfo->fd, &des);
			assert(des.status == 1);

			if (g_serv_info.setting_server_fd == portinfo->fd) {
				x_printf(E, "setting_server failed.");
				g_serv_info.setting_server_fd = 0;
				fdman_list_remove(SETTING_SERVER, portinfo->fd);
			} else {
				x_printf(S, "this fd:%d is not belong to SETTING_SERVER.", portinfo->fd);
			}

			commapi_close(commctx, portinfo->fd);
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
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);

	switch (portinfo->stat)
	{
		case FD_INIT:	// connected.
		{
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = LOGIN_SERVER;
			make_uuid(des.uuid);
			fdman_array_fill_fd(portinfo->fd, &des);

			struct fd_node node = {};
			node.fd = portinfo->fd;
			node.status = 1;
			fdman_list_push_back(LOGIN_SERVER, &node);
			
			g_serv_info.login_server_fd = portinfo->fd;
		}
		break;

		case FD_CLOSE:	// closed.
		{
			struct fd_descriptor des;
			fdman_array_remove_fd(portinfo->fd, &des);
			assert(des.status == 1);

			if (g_serv_info.login_server_fd == portinfo->fd) {
				x_printf(E, "login_server failed.");
				g_serv_info.login_server_fd = 0;
				fdman_list_remove(LOGIN_SERVER, portinfo->fd);
			} else {
				x_printf(S, "this fd:%d is not belong to LOGIN_SERVER.", portinfo->fd);
			}

			commapi_close(commctx, portinfo->fd);
			break;
		}

		default:
			break;
	}
}

