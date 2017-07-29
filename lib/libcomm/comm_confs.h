#pragma once

#define EPOLL_SIZE      (1082 * 100)	/* 允许EPOLL能够监听的描述符的最大个数 */
#define LISTEN_SIZE     10		/* 允许监听fd的最大个数 */

#define MAX_COMM_FRAMES 13		/* 允许一个包最大的总帧数 */

#define QUEUE_NODES     1024		/* 队列里面可以存放多少个节点的数据 */

#define COMM_READ_MIOU  (1024 * 1024)	/* 一次性最多读取数据的字节数 */

#define EPOLLTIMEOUTED  20		/* epoll_wait的超时事件 以毫秒(ms)为单位 1s = 1000ms*/
//#define EPOLLTIMEOUTED  2000000		/* epoll_wait的超时事件 以毫秒(ms)为单位 1s = 1000ms*/

#define DELAY_RECONNECT_INTERVAL	2000/*2s*/
