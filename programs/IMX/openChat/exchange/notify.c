#include <uuid/uuid.h>

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

void client_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			{
				printf("server here is accept : %d\n", socket);
				struct fd_descriptor des = {};
				des.status = 1;
				des.obj = CLIENT;
				make_uuid(des.uuid);
				fdman_array_fill_fd(socket, &des);

				send_status_msg(socket, STEP_INIT);
			}
			break;

		case STEP_ERRO:
			printf("server here is error : %d\n", socket);
			commapi_close(commctx, socket);
			break;

		case STEP_WAIT:
			printf("server here is wait : %d\n", socket);
			break;

		case STEP_STOP:
			{
				printf("server here is close : %d\n", socket);
				struct fd_descriptor des = {};
				fdman_array_remove_fd(socket, &des);

				erase_client(socket);
				x_printf(D, "errase client fd:%d.", socket);

				send_status_msg(socket, STEP_STOP);

			}
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

void message_gateway_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	if (g_serv_info.commctx != commctx) {
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_descriptor des = {};
				des.status = 1;
				des.obj = MESSAGE_GATEWAY;
				make_uuid(des.uuid);
				fdman_array_fill_fd(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_push_back(MESSAGE_GATEWAY, &node);

				g_serv_info.message_gateway_fd = socket;
			}
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;


		case STEP_STOP:
			// closed.
			{
				printf("client here is close : %d\n", socket);
				struct fd_descriptor des;
				fdman_array_remove_fd(socket, &des);
				assert(des.status == 1);

				if (g_serv_info.message_gateway_fd == socket) {
					x_printf(E, "message_server failed.");
					g_serv_info.message_gateway_fd = 0;
					fdman_list_remove(MESSAGE_GATEWAY, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to MESSAGE_GATEWAY.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}

void setting_server_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	if (g_serv_info.commctx != commctx) {
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_descriptor des = {};
				des.status = 1;
				des.obj = SETTING_SERVER;
				make_uuid(des.uuid);
				fdman_array_fill_fd(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_push_back(SETTING_SERVER, &node);

				g_serv_info.setting_server_fd = socket;
			}
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;


		case STEP_STOP:
			// closed.
			{
				printf("client here is close : %d\n", socket);
				struct fd_descriptor des;
				fdman_array_remove_fd(socket, &des);
				assert(des.status == 1);

				if (g_serv_info.setting_server_fd == socket) {
					x_printf(E, "setting_server failed.");
					g_serv_info.setting_server_fd = 0;
					fdman_list_remove(SETTING_SERVER, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to SETTING_SERVER.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}

void login_server_event_notify(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	if (g_serv_info.commctx != commctx) {
		x_printf(E, "callback commctx not equal. g_serv_info.commctx:%p, commctx:%p", g_serv_info.commctx, commctx);
	}
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_descriptor des = {};
				des.status = 1;
				des.obj = LOGIN_SERVER;
				make_uuid(des.uuid);
				fdman_array_fill_fd(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_push_back(LOGIN_SERVER, &node);

				g_serv_info.login_server_fd = socket;
			}
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;


		case STEP_STOP:
			// closed.
			{
				printf("client here is close : %d\n", socket);
				struct fd_descriptor des;
				fdman_array_remove_fd(socket, &des);
				assert(des.status == 1);

				if (g_serv_info.login_server_fd == socket) {
					x_printf(E, "login_server failed.");
					g_serv_info.login_server_fd = 0;
					fdman_list_remove(LOGIN_SERVER, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to LOGIN_SERVER.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}
