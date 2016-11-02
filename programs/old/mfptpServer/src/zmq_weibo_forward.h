/* 文件: zmq_weibo_forward.h
 *   版权:
 *   描述: 本文件提供weibo转发的接口
 *   历史: 2015/3/22 新文件建立by l00167671/luokaihui
 */

#ifndef __ZMQ_WEIBO_FORWARD_H__
#define __ZMQ_WEIBO_FORWARD_H__

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <zmq.h>
#include <unistd.h>

void *mfptp_usr_weibo_forward(void *arg);

void *mfptp_gp_weibo_forward(void *arg);

/*
 * 函数名:mfptp_edit_user_channel_info
 * 功能:编译用户和频道信息
 * 参数:oper　操作类型，'1' 是用户加入某个频道，'2'是用户脱离某个频道, imei用户的imei, channel 用户的频道
 * 返回值:目前只返回TRUE
 */
int mfptp_edit_user_channel_info(unsigned char oper, char *imei, char *channel);
#endif

