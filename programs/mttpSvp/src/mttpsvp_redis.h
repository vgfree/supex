 /* @author: qianye@mirrtalk.com */

#ifndef MTTPSVP_REDIS_H
#define MTTPSVP_REDIS_H

void mttpsvp_redis_init(const char *hostname, int port);
void mttpsvp_redis_destory(); 

int mttpsvp_redis_check_gpstoken(const char *mirrtalk_id, const char *gpdtoken, size_t gpdtoken_len);

#endif /* MTTPSVP_REDIS_H */
