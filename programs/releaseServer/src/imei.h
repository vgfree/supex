#pragma once

typedef struct _IMEI_USER
{
	char    imei[16];
	char    user[16];
} IMEI_USER;

extern IMEI_USER        *g_imei_tab;
extern int              g_imei_cnt;

extern int imei_init(void);

extern void imei_term(void);

extern int user_search(char *imei, char *user);

