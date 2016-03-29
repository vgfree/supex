#ifndef __MFPTP_CALLBACK_H__
#define __MFPTP_CALLBACK_H__

/* 名      称: mfptp_drift_out_func
 *  功      能: 把mfptp 协议中的消息转化成ZMQ消息转发出去
 *  参      数:
 *  返回值:
 *  修      改: 新生成函数l00167671 at 2015/2/28
 */
void *mfptp_drift_out_callback(void *data);

/* 名      称: mfptp_auth_func
 *  功      能: 对于登录消息进行验证
 *  参      数:
 *  返回值:
 *  修
 *  改: 新生成函数l00167671 at 2015/2/28, 暂不进行任何验证
 */
void *mfptp_auth_callback(void *data);
#endif

