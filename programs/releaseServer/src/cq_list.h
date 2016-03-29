#pragma once

// #define CQ_LEN          10       /*队列长度*/
#define CQ_LEN 500			/*队列长度*/
int cqu_init(void **cq);

int cqu_push(void *cq, void *data);

int cqu_pull(void *cq, void **data);

