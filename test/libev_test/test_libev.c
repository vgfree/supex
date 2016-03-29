/*****************************************************************************
 * Copyright(c) shanghai... 2015-2100
 * Filename: main.c
 * Author: shaozhenyu 18217258834@163.com
 * History:
 *        created by shaozhenyu 2015-10-12
 * Description:
 *
 ******************************************************************************/
#include <ev.h>
#include <stdio.h>
#include <signal.h>
#include <sys/unistd.h>

ev_io io_w;

void io_action(struct ev_loop *loop, ev_io *io_w, int e)
{
	printf("io_action -------------\n");
	ev_io_stop(loop, io_w);
}

int main()
{
	struct ev_loop *main_loop = ev_default_loop(0);

	struct async_ctx *ac = NULL;

	ac = async_initial(main_loop, 1, NULL, NULL, NULL, 5);

	ev_io_init(&io_w, io_action, STDIN_FILENO, EV_READ);

	ev_io_start(main_loop, &io_w);

	ev_run(main_loop, 0);
	printf("begin............\n");
	return 0;
}

