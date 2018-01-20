#include <uuid/uuid.h>

#include "libmini.h"
#include "exc_sockfd_manager.h"
#include "exc_message_dispatch.h"
#include "exc_status.h"
#include "exc_event_notify.h"

void make_uuid(char *dst)
{
	uuid_t  uuid;
	uuid_generate_time(uuid);
	uuid_unparse(uuid, dst);
}

void exc_event_notify_from_client(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			{
				printf("server here is accept : %d\n", socket);
				struct fd_info des = {};
				des.status = 1;
				des.type = CLIENT_ROUTER;
				make_uuid(des.uuid);
				fdman_slot_set(socket, &des);

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
				fdman_slot_del(socket);

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

void exc_event_notify_from_message(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_info des = {};
				des.status = 1;
				des.type = MESSAGE_ROUTER;
				make_uuid(des.uuid);
				fdman_slot_set(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_add(MESSAGE_ROUTER, &node);

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
				fdman_slot_del(socket);

				if (g_serv_info.message_gateway_fd == socket) {
					x_printf(E, "message_server failed.");
					g_serv_info.message_gateway_fd = 0;
					fdman_list_del(MESSAGE_ROUTER, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to MESSAGE_ROUTER.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}

void exc_event_notify_from_control(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_info des = {};
				des.status = 1;
				des.type = CONTROL_ROUTER;
				make_uuid(des.uuid);
				fdman_slot_set(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_add(CONTROL_ROUTER, &node);

				g_serv_info.control_gateway_fd = socket;
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
				fdman_slot_del(socket);

				if (g_serv_info.control_gateway_fd == socket) {
					x_printf(E, "setting_server failed.");
					g_serv_info.control_gateway_fd = 0;
					fdman_list_del(CONTROL_ROUTER, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to CONTROL_ROUTER.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}

void exc_event_notify_from_login(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
			{
				printf("client here is connect : %d\n", socket);
				struct fd_info des = {};
				des.status = 1;
				des.type = LOGIN_ROUTER;
				make_uuid(des.uuid);
				fdman_slot_set(socket, &des);

				struct fd_node node = {};
				node.fd = socket;
				node.status = 1;
				fdman_list_add(LOGIN_ROUTER, &node);

				g_serv_info.login_gateway_fd = socket;
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
				fdman_slot_del(socket);

				if (g_serv_info.login_gateway_fd == socket) {
					x_printf(E, "login_server failed.");
					g_serv_info.login_gateway_fd = 0;
					fdman_list_del(LOGIN_ROUTER, socket);
				} else {
					x_printf(S, "this fd:%d is not belong to LOGIN_ROUTER.", socket);
				}
				break;
			}

		default:
			printf("unknow!\n");
			break;
	}
}
