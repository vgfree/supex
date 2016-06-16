#ifndef __KV_SPEED_H__
#define __KV_SPEED_H__

#include "rr_def.h"

#define ERR_IMEI        -1
#define SUC_IMEI        0
#define NIL_IMEI        1

typedef struct kv_speed_t
{
	// short flag;        //确实是否为停车
	// long  str_time;    //速度为0,开始时间
	// long  end_time;    //速度为0,结束时间
	// int   count;       //速度为0,次数
	char    tokenCode[IMEI_LEN + 1];/*当前tokenCode*/
	int     sign;
} kv_speed;

int get_speed_from_kv(long IMEI, kv_speed *kv_speed_ex);

int set_speed_to_kv(long IMEI, kv_speed *kv_speed_ex);
#endif /* ifndef __KV_SPEED_H__ */

