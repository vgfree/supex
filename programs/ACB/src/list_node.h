#ifndef __LIST_NODE_H__
#define __LIST_NODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/*数据节点*/
struct d_node
{
	char            IMEI[20];
	time_t          GPSTime;
	double          log;
	double          lat;
	long            log_lat;

	struct d_node   *d_next;
	struct d_node   *d_prev;
};
/*方位节点*/
struct l_node
{
	long            Log_lat;

	struct l_node   *l_next;
	struct l_node   *l_prev;

	struct d_node   *d_next;
};
/*时间节点*/
struct t_node
{
	time_t          Time;

	struct l_node   *l_next;

	struct t_node   *t_next;
	struct t_node   *t_prev;
};

/*
 *功能：将时间节点插入时间链表，头插
 *参数：时间链表头节点指针(t_head),插入时间链表的节点指针(t_new)
 */
void time_node_insert(struct t_node *t_head, struct t_node *t_new);

/*
 *功能：将方位节点插入方位链表，头插
 *参数：方位链表头节点指针(l_head),插入方位链表的节点指针(l_new)
 */
void location_node_insert(struct l_node *l_head, struct l_node *l_new);

/*
 *功能：将数据节点插入数据链表，头插
 *参数：数据链表头节点(d_head),插入数据链表的节点(d_new)
 */
void data_node_insert(struct d_node *d_head, struct d_node *d_new);

/*
 *功能：给时间节点动态分配空间，并使成员指针都指向自己，开辟空间失败则报错并退出
 *参数：时间节点的二级指针
 */
void time_node_init(struct t_node **t_new);

/*
 *功能：给方位节点动态分配空间，并使成员指针都指向自己，开辟空间失败则报错并退出
 *参数：方位节点的二级指针
 */
void location_node_init(struct l_node **l_new);

/*
 *功能：给数据节点动态分配空间，并使成员指针都指向自己，开辟空间失败则报错并退出
 *参数：数据节点的二级指针
 */
void data_node_init(struct d_node **d_new);

/*
 *功能：删除数据节点，释放节点空间
 *参数：数据节点的指针
 */
void del_data_node(struct d_node *p_D__node);	//

/*
 *功能：删除方位节点，释放节点空间
 *参数：方位节点的指针
 */
void del_location_node(struct l_node *p_L__node);

/*
 *功能：删除时间节点，释放节点空间
 *参数：时间节点的指针
 */
void del_time_node(struct t_node *p_T__node);

/*
 *功能：遍历数据链表检测与数据节点距离相近的节点并调用data_to_command(),import_to_redis()函数
 *参数：数据量表的头节点指针(l_head)数据节点的指针(p_D_node)，传给import_to_redis()函数的参数loop，host，port
 */
void check_data_distance(struct l_node *l_head, struct d_node *p_D_node, double distance, void *loop, char host[], unsigned short port);

/*
 *功能：遍历方位链表找方位相同的节点，没有则创建方位节点并插入方位链表，找到则将数据节点插入数据链表并调用check_data_distance()函数
 *参数：时间链表头节点指针t_head,数据节点指针d_new，传给import_to_redis()的参数loop,host,port
 */
void check_location_node(struct t_node *p_t_head, struct d_node *d_new, double distance, void *loop, char host[], unsigned short port);

/*
 *功能：遍历时间链表找时间相同的节点，没有则创建时间节点并插入时间链表，找到则调用check_location_node()函数，打印遍历时间travel_time
 *参数：时间链表头节点指针，数据节点指针，传给import_to_redis()的参数loop,host,port
 */
void check_data_time(struct t_node *t_head, struct d_node *d_new, double distance, void *loop, char host[], unsigned short port);

/*
 *功能：初始化数据节点，调用check_data_time()函数
 *参数：时间链表头节点指针t_head,数据节点所需数据Imei,Time,Log,Lat,传给import_to_redis()的参数loop,host,port
 */
void creat_data_node(struct t_node *t_head, char Imie[], time_t Time, double Log, double Lat, double distance, void *loop, char host[], unsigned short port);

/*
 *功能：初始化方位节点,创建对应的数据链表头节点
 *参数：方位链表头节点指针l_head,数据节点指针d_new
 */
void creat_location_node(struct l_node *l_head, struct d_node *d_new);

/*
 *功能：初始化时间节点，创建对应的方位链表头节点并调用creat_location_node()函数
 *参数：时间链表头节点指针t_head,数据节点指针d_new
 */
void creat_time_node(struct t_node *thead, struct d_node *d_new);

/*
 *功能：将数据字符串分拆成数据，并调用creat_data_node
 *参数：data数据字符串,时间链表头节点指针t_head,传给import_to_redis()的参数loop,host,port
 */
void data_to_redis(struct t_node *t_head, char *data, void *loop, char host[], unsigned short port);

/*
 *功能：将数据拼成redis命令字符串
 *参数：Imei_1设备编号,Imei_2设备编号,command存放命令的字符串,Time数据时间
 */
void data_to_command(char Imei_1[], char Imei_2[], char command[], time_t Time);

/*
 *功能：将command作为redis命令，写入redis中
 *参数：命令command，redis地址host，redis端口port
 */
int import_to_redis(char command[], void *loop, char host[], unsigned short port);
#endif	/* ifndef __LIST_NODE_H__ */

