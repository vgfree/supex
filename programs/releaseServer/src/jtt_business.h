#pragma once

typedef struct _NODE
{
	char    userid[16];
	void    *data;
} NODE;

int jtt809_out_once_init(void);

int jtt809_data_handle(const void *data);

