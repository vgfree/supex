#include <uuid/uuid.h>

#include "libmini.h"
#include "exc_sockfd_manager.h"
#include "exc_message_dispatch.h"
#include "exc_status.h"
#include "exc_event_notify.h"

void make_uuid(char *dst)
{
	uuid_t uuid;

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
			printf("exchange here is accept client: %d\n", socket);
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
			printf("exchange here is wait client: %d\n", socket);
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

void exc_event_notify_from_stream(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			// connected.
		{
			printf("exchange here is connect streamGateway: %d\n", socket);
			struct fd_info des = {};
			des.status = 1;
			des.type = STREAM_ROUTER;
			make_uuid(des.uuid);
			fdman_slot_set(socket, &des);

			struct fd_node node = {};
			node.fd = socket;
			node.status = 1;
			fdman_list_add(STREAM_ROUTER, &node);

			g_serv_info.stream_gateway_fd = socket;
		}
		break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("exchange here is wait streamGateway: %d\n", socket);
			break;

		case STEP_STOP:
			// closed.
		{
			printf("client here is close : %d\n", socket);
			fdman_slot_del(socket);

			if (g_serv_info.stream_gateway_fd == socket) {
				x_printf(E, "streamGateway failed.");
				g_serv_info.stream_gateway_fd = 0;
				fdman_list_del(STREAM_ROUTER, socket);
			} else {
				x_printf(S, "this fd:%d is not belong to STREAM_ROUTER.", socket);
			}

			break;
		}

		default:
			printf("unknow!\n");
			break;
	}
}

void exc_event_notify_from_manage(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
		{
			printf("exchange here is connect manageGateway: %d\n", socket);
			struct fd_info des = {};
			des.status = 1;
			des.type = MANAGE_ROUTER;
			make_uuid(des.uuid);
			fdman_slot_set(socket, &des);

			struct fd_node node = {};
			node.fd = socket;
			node.status = 1;
			fdman_list_add(MANAGE_ROUTER, &node);

			g_serv_info.manage_gateway_fd = socket;
		}
		break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("exchange here is wait manageGateway: %d\n", socket);
			break;

		case STEP_STOP:
			// closed.
		{
			printf("client here is close : %d\n", socket);
			fdman_slot_del(socket);

			if (g_serv_info.manage_gateway_fd == socket) {
				x_printf(E, "manageGateway failed.");
				g_serv_info.manage_gateway_fd = 0;
				fdman_list_del(MANAGE_ROUTER, socket);
			} else {
				x_printf(S, "this fd:%d is not belong to MANAGE_ROUTER.", socket);
			}

			break;
		}

		default:
			printf("unknow!\n");
			break;
	}
}

void exc_event_notify_from_status(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	assert(g_serv_info.commctx == commctx);
	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			// connected.
		{
			printf("exchange here is connect statusGateway: %d\n", socket);
			struct fd_info des = {};
			des.status = 1;
			des.type = STATUS_ROUTER;
			make_uuid(des.uuid);
			fdman_slot_set(socket, &des);

			struct fd_node node = {};
			node.fd = socket;
			node.status = 1;
			fdman_list_add(STATUS_ROUTER, &node);

			g_serv_info.status_gateway_fd = socket;
		}
		break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("exchange here is wait statusGateway: %d\n", socket);
			break;

		case STEP_STOP:
			// closed.
		{
			printf("client here is close : %d\n", socket);
			fdman_slot_del(socket);

			if (g_serv_info.status_gateway_fd == socket) {
				x_printf(E, "statusGateway failed.");
				g_serv_info.status_gateway_fd = 0;
				fdman_list_del(STATUS_ROUTER, socket);
			} else {
				x_printf(S, "this fd:%d is not belong to STATUS_ROUTER.", socket);
			}

			break;
		}

		default:
			printf("unknow!\n");
			break;
	}
}

