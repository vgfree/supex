#pragma once

#define TYPE_JTT809     1		/*协议类型*/
#define TYPE_HTTP       2		/*协议类型*/

extern int g_thread_cnt;

int grp_thread_init(void);

int grp_thread_boot(void);

int grp_thread_push(char type, void *data);

