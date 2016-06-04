#include "list_node.h"
#include "async_tasks/async_api.h"
#include "pools/xpool.h"
#include "pool_api/conn_xpool_api.h"
#include "redis_api/redis_status.h"
#include "redis_parse.h"
#define REDIS_ERR       -1
#define REDIS_OK        0

// 遍历时间
int travel_time = 0;

void time_node_insert(struct t_node *t_head, struct t_node *t_new)
{
	t_new->t_next = t_head->t_next;
	t_head->t_next->t_prev = t_new;
	t_new->t_prev = t_head;
	t_head->t_next = t_new;
}

void location_node_insert(struct l_node *l_head, struct l_node *l_new)
{
	l_new->l_next = l_head->l_next;
	l_head->l_next->l_prev = l_new;
	l_new->l_prev = l_head;
	l_head->l_next = l_new;
}

void data_node_insert(struct d_node *d_head, struct d_node *d_new)
{
	d_new->d_next = d_head->d_next;
	d_head->d_next->d_prev = d_new;
	d_new->d_prev = d_head;
	d_head->d_next = d_new;
}

void time_node_init(struct t_node **t_new)
{
	(*t_new) = malloc(sizeof(struct t_node));

	if (*t_new == NULL) {
		// x_printf(E,"time node malloc error");
		exit(-1);
	}

	(*t_new)->t_next = (*t_new);
	(*t_new)->t_prev = (*t_new);
	(*t_new)->l_next = (struct l_node *)(*t_new);
}

void location_node_init(struct l_node **l_new)
{
	(*l_new) = malloc(sizeof(struct l_node));

	if (*l_new == NULL) {
		// x_printf(E,"location node malloc error");
		exit(-1);
	}

	(*l_new)->d_next = (struct d_node *)(*l_new);
	(*l_new)->l_prev = (*l_new);
	(*l_new)->l_next = (*l_new);
}

void data_node_init(struct d_node **d_new)
{
	(*d_new) = malloc(sizeof(struct d_node));

	if (*d_new == NULL) {
		// x_printf(E,"data node malloc error");
		exit(-1);
	}

	(*d_new)->d_next = (*d_new);
	(*d_new)->d_prev = (*d_new);
}

void del_data_node(struct d_node *p_D_node)
{
	p_D_node->d_next->d_prev = p_D_node->d_prev;
	p_D_node->d_prev->d_next = p_D_node->d_next;
	p_D_node->d_next = NULL;
	p_D_node->d_prev = NULL;

	if (p_D_node != NULL) {
		free(p_D_node);
		p_D_node = NULL;
	}
}

void del_location_node(struct l_node *p_L_node)
{
	struct d_node   *head = p_L_node->d_next;
	struct d_node   *temp = NULL;

	// 遍历删除方位节点下的数据链表
	for (temp = head->d_next; temp != head; temp = temp->d_next) {
		temp = temp->d_prev;
		del_data_node(temp->d_next);
	}

	//删除数据链表头节点
	del_data_node(p_L_node->d_next);
	//删除方位节点
	p_L_node->d_next = NULL;
	p_L_node->l_next->l_prev = p_L_node->l_prev;
	p_L_node->l_prev->l_next = p_L_node->l_next;
	p_L_node->l_next = NULL;
	p_L_node->l_prev = NULL;

	if (p_L_node != NULL) {
		// x_printf(D,"del_Log_Lat_node:%d",p_L_node->Log_lat);
		free(p_L_node);
		p_L_node = NULL;
	}
}

void del_time_node(struct t_node *p_T_node)
{
	struct l_node   *head = p_T_node->l_next;
	struct l_node   *temp = NULL;

	// 遍历删除时间节点下的方位链表
	for (temp = head->l_next; temp != head; temp = temp->l_next) {
		temp = temp->l_prev;
		del_location_node(temp->l_next);
	}

	//删除方位链表头节点
	head->l_next = NULL;
	head->l_prev = NULL;

	if (head != NULL) {
		free(head);
		head = NULL;
	}

	//删除时间节点
	p_T_node->l_next = NULL;
	p_T_node->t_next->t_prev = p_T_node->t_prev;
	p_T_node->t_prev->t_next = p_T_node->t_next;
	p_T_node->t_next = NULL;
	p_T_node->t_prev = NULL;

	if (p_T_node != NULL) {
		// x_printf(D,"del_time_node:%d",p_T_node->Time);
		free(p_T_node);
		p_T_node = NULL;
	}
}

void creat_location_node(struct l_node *l_head, struct d_node *d_new)
{
	struct l_node *l_new = NULL;

	location_node_init(&l_new);
	l_new->Log_lat = d_new->log_lat;
	location_node_insert(l_head, l_new);
	// 创建数据链表头节点
	struct d_node *d_head = NULL;
	data_node_init(&d_head);
	l_new->d_next = d_head;
}

void creat_time_node(struct t_node *t_head, struct d_node *d_new)
{
	// x_printf(D,"creat_time_node:%ld\n",d_new->GPSTime);
	struct t_node *t_new = NULL;

	time_node_init(&t_new);

	t_new->Time = d_new->GPSTime;
	time_node_insert(t_head, t_new);
	// 创建方位链表头节点
	struct l_node *l_head = NULL;
	location_node_init(&l_head);
	t_new->l_next = l_head;

	creat_location_node(l_head, d_new);
}

int import_to_redis(char command[], void *loop, char host[], unsigned short port)
{

	struct xpool    *cpool = conn_xpool_find(host, port);
	struct async_api *api = async_api_initial(loop, 1, true, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL);
	// x_printf(D,"import_to_redis: %s\n",command);

	if (api && cpool) {
		/*data*/
		char    *proto;
		int     ok = cmd_to_proto(&proto, command);

		if (ok == REDIS_ERR) {
			async_api_distory(api);
			return -1;
		}

		/*send*/
		struct command_node *cmd = async_api_command(api, PROTO_TYPE_REDIS, cpool, proto, strlen(proto), NULL, NULL);
		free(proto);
		if (cmd == NULL) {
			async_api_distory(api);
			return -1;
		}


		async_api_startup(api);
		return 0;
	}

	return -1;
}

void data_to_command(char Imei_1[], char Imei_2[], char command[], time_t Time)
{
	char    IMEI_1[25];
	char    IMEI_2[25];

	memset(IMEI_1, 0, sizeof(IMEI_1));
	memset(IMEI_2, 0, sizeof(IMEI_2));

	strcpy(IMEI_1, Imei_1);
	IMEI_1[strlen(IMEI_1)] = '\0';
	strcpy(IMEI_2, Imei_2);
	IMEI_2[strlen(IMEI_2)] = '\0';

	struct tm       *time;
	char            command_k[20];
	char            command_v[50];

	time = gmtime(&Time);
	int     year = time->tm_year + 1900;
	int     mon = time->tm_mon + 1;
	int     day = time->tm_mday;
	int     sec = time->tm_sec;
	int     min = time->tm_min;
	int     hour = time->tm_hour;

	command_k[0] = year / 1000 + 48;
	command_k[1] = year % 1000 / 100 + 48;
	command_k[2] = year % 1000 % 100 / 10 + 48;
	command_k[3] = year % 10 + 48;					// 年份

	if (mon < 10) {
		command_k[4] = 0 + 48;
		command_k[5] = mon + 48;
	} else {
		command_k[4] = mon / 10 + 48;
		command_k[5] = mon % 10 + 48;
	}							// 月份

	if (day < 10) {
		command_k[6] = 0 + 48;
		command_k[7] = day + 48;
	} else {
		command_k[6] = day / 10 + 48;
		command_k[7] = day % 10 + 48;
	}							// 天数

	int     c_sec = (sec + min * 60 + hour * 3600);
	int     s_sec = (c_sec - (c_sec % 300)) + 300;

	if (s_sec < 1000) {
		command_k[8] = '0';
		command_k[9] = '0';
		command_k[10] = s_sec / 100 + 48;
	}

	if ((s_sec < 10000) && (s_sec >= 1000)) {
		command_k[8] = '0';
		command_k[9] = s_sec / 1000 + 48;
		command_k[10] = s_sec % 1000 / 100 + 48;
	}

	if ((s_sec < 100000) && (s_sec >= 10000)) {
		command_k[8] = s_sec / 10000 + 48;
		command_k[9] = s_sec % 10000 / 1000 + 48;
		command_k[10] = s_sec % 1000 / 100 + 48;
	}							// 当天零点到Time的秒数，每300秒一组

	command_k[11] = '\0';
	strcat(command_k, "00:ACBset \0");			// KEY组装

	memset(command_v, 0, sizeof(command_v));

	if (strcmp(IMEI_1, IMEI_2) < 0) {
		IMEI_1[strlen(IMEI_1)] = ',';
		IMEI_1[strlen(IMEI_1)] = '\0';
		strcat(command_v, IMEI_1);

		IMEI_2[strlen(IMEI_2)] = '\0';
		strcat(command_v, IMEI_2);
		command_v[strlen(command_v)] = '\0';
	} else {
		IMEI_2[strlen(IMEI_2)] = ',';
		IMEI_2[strlen(IMEI_2)] = '\0';
		strcat(command_v, IMEI_2);

		IMEI_1[strlen(IMEI_1)] = '\0';
		strcat(command_v, IMEI_1);
		command_v[strlen(command_v)] = '\0';
	}							// VALUE组装

	strcpy(command, "SADD ");
	strcat(command, command_k);
	strcat(command, command_v);					// 将命令，KEY，VALUE组装成字符串
									// eg:"SADD 2015092500300ACBset 123456789012344,123456789012345"
	// x_printf(I,"command:%s",command);
}

void check_data_distance(struct l_node *p_L_node, struct d_node *p_D_node, double distance, void *loop, char host[], unsigned short port)
{
	char command[100];

	memset(command, 0, sizeof(command));
	struct d_node   *head = p_L_node->d_next;
	struct d_node   *temp = NULL;

	// x_printf(D,"check_data_distance:%s",p_D_node->IMEI);
	// 遍历数据链表找距离相近的数据节点，找到调用data_to_command，import_to_redis，自加travel_time
	for (temp = head->d_next; temp != head; temp = temp->d_next) {
		travel_time++;

		if (strcmp(p_D_node->IMEI, temp->IMEI)) {
			if ((p_D_node->log - temp->log < distance) && (p_D_node->log - temp->log > (distance - 2 * distance))) {
				if ((p_D_node->lat - temp->lat < distance) && (p_D_node->lat - temp->lat > (distance - 2 * distance))) {
					data_to_command(p_D_node->IMEI, temp->IMEI, command, p_D_node->GPSTime);

					if (!import_to_redis(command, loop, host, port)) {
						// x_printf(I,"import_to_redis:%s  --------------------------  SUCCESS",p_D_node->IMEI);
					} else {
						// x_printf(E,"import_to_redis:%s  --------------------------  FAIL",p_D_node->IMEI);
						// x_printf(I,"command:%s",command);
					}

					memset(command, 0, sizeof(command));
				}
			}
		}
	}
}

void check_location_node(struct t_node *p_T_node, struct d_node *d_new, double distance, void *loop, char host[], unsigned short port)
{
	struct l_node   *head = p_T_node->l_next;
	struct l_node   *temp = NULL;

	// 检查location链表有没有数据节点，没有就创建一个
	if (head->l_next == head) {
		creat_location_node(temp, d_new);
	}

	// x_printf(D,"check_location_node::%d",d_new->log_lat);
	// 遍历方位链表找对应的方位节点，找到调用check_data_distance，没有则创建时间节点并插入时间链表，自加travel_time
	for (temp = head->l_next; temp != head; temp = temp->l_next) {
		travel_time++;

		if (temp->Log_lat == d_new->log_lat) {
			data_node_insert(temp->d_next, d_new);
			check_data_distance(temp, d_new, distance, loop, host, port);
			d_new->log_lat = 0;
		} else if ((temp == head->l_prev) && (d_new->log_lat != 0)) {
			creat_location_node(head, d_new);
		}
	}
}

void check_data_time(struct t_node *t_head, struct d_node *d_new, double distance, void *loop, char host[], unsigned short port)
{
	struct t_node   *temp = NULL;
	int             time_node_num = 0;

	// 检测data节点时间戳是否过期，过期删除,检查time链表有没有数据节点，没有就创建一个
	if ((d_new->GPSTime + 60) < time(NULL)) {
		del_data_node(d_new);
	} else if (t_head->t_prev == t_head) {
		creat_time_node(t_head, d_new);
	}

	// 遍历时间链表找对应的时间节点，找到调用check_location_node，没有则创建时间节点并插入时间链表，自加travel_time
	for (temp = t_head->t_prev; temp != t_head; temp = temp->t_prev) {
		time_node_num++;
		travel_time++;

		// 检测节点时间戳是否过期，过期删除
		if ((temp->Time + 60) < time(NULL)) {
			temp = temp->t_next;
			del_time_node(temp->t_prev);
		} else if (temp->Time == d_new->GPSTime) {
			check_location_node(temp, d_new, distance, loop, host, port);
			d_new->GPSTime = 0;
		} else if ((temp == t_head->t_next) && (d_new->GPSTime != 0)) {
			creat_time_node(t_head, d_new);
		}
	}

	// 打印遍历时间和时间链表长度
	// x_printf(I,"TIME_NODE_NUM = %d",time_node_num);
	// x_printf(I,"travel_time:%d",travel_time);

	travel_time = 0;
}

void creat_data_node(struct t_node *t_head, char Imei[], time_t Time, double Log, double Lat, double distance, void *loop, char host[], unsigned short port)
{
	struct d_node *d_new = NULL;

	data_node_init(&d_new);
	// 将数据写入节点
	strcpy(d_new->IMEI, Imei);
	d_new->IMEI[strlen(d_new->IMEI)] = '\0';
	d_new->GPSTime = Time;
	d_new->log = Log;
	d_new->lat = Lat;
	// d_new->log_lat = (int)(d_new->log * 10) * 1000 + (int)(d_new->lat * 10);
	d_new->log_lat = (long)(d_new->log * 1000) * 100000 + (long)(d_new->lat * 1000);
	x_printf(D, "creat_data_nodelog_lat %ld\n", d_new->log_lat);
	check_data_time(t_head, d_new, distance, loop, host, port);
}

void data_to_redis(struct t_node *t_head, char *data, void *loop, char host[], unsigned short port)
{
	char IMEI[25];

	memset(IMEI, 0, sizeof(IMEI));
	char temp[20];
	memset(temp, 0, sizeof(temp));
	char dst[10];
	memset(dst, 0, sizeof(dst));
	time_t  Time;
	double  log;
	double  lat;

	char    *p;
	char    *q;
	char    *t;
	int     i;

	// x_printf(D,"data_to_redis start'data: %s\n",data);

	// 取distance
	strncpy(dst, data, 5);
	double distance = (double)atof(dst);
	// strcpy(data,data+5);
	data = data + 5;
	// 数据包数据个数

	int data_num = data[0] - 48;
	// 取IMEI
	p = strchr(data, ':');
	q = strchr(p, '/');
	strncpy(IMEI, p + 1, q - p - 1);

	// x_printf(D,"data_to_redis");
	// 循环取出字符串中的数据并调用creat_data_node
	for (i = 0; i < data_num; i++) {
		// 取time并转换成time_t类型
		p = strchr(q, ',');
		strncpy(temp, q + 1, p - q - 1);
		Time = (time_t)atoi(temp);
		// 取log并转换成double类型
		memset(temp, 0, sizeof(temp));
		q = strchr(p + 1, ',');
		strncpy(temp, p + 1, q - p - 1);
		log = atof(temp);
		// 取lat并转换成double类型
		memset(temp, 0, sizeof(temp));
		p = strchr(q, '/');
		strncpy(temp, q + 1, p - q - 1);
		lat = atof(temp);

		t = p;
		p = q;
		q = t;

		creat_data_node(t_head, IMEI, Time, log, lat, distance, loop, host, port);
		// x_printf(D,"creat_data_node::%s",IMEI);
	}
}

