#include "proto.h"



static struct list *listcreate();
static void listInsert(struct list *list, const char *value, int len);
static void listPrint(struct list *list);
static void listDestroy(struct list *list);



struct list * proto_init()
{
	struct list *list = listcreate();
	return list;
}

void proto_append(struct list *list, const char *data, int len)
{
	listInsert(list, data, len);
}

char *proto_pack(struct list *list)
{
	char    *buf = malloc(list->text_count+sizeof(int));
	int *set = (int *)buf;
	int i;

	set[0] = list->len;

	struct listNode *node = list->head;
	for (i = 1; i <= list->len; i++) {
		set[i] = node->node_len;
		node = node->next;	
	}	
	char *p = buf + (list->len + 1)*sizeof(int);

	struct listNode *nd = list->head;
	while (nd != NULL) {
		memcpy(p, nd->value, nd->node_len);
		p += nd->node_len;
		nd = nd->next;
	}

	return buf;
}

void proto_destroy(struct list *list)
{
	listDestroy(list);
}

void proto_parse(struct list *list, const char *t)
{
	long long ll, l;
	int i;
	const char *p = t;

	ll = *(int*)t;
	p = p + sizeof(int) + ll*sizeof(int);
	t += sizeof(int);

	for (i = 0; i < ll; i++) {
		l = *(int*)t;
		t += sizeof(int);
		listInsert(list, p, l);
		p += l;
	}
}

void proto_get(struct list *list)
{
	listPrint(list);
}

void strfree(char *str)
{
	free(str);
	str = NULL;
}

list *listcreate(void)
{
	struct list *l = malloc(sizeof(struct list));
	assert(l);
	l->head        = NULL;
	l->text_count  = 0;
	l->len         = 0;

	return l;
}

static void listInsert(struct list *list, const char *value, int len)
{
	struct listNode *p;
	struct listNode *node = malloc(sizeof(struct listNode));
	assert(node);
	node->value = malloc(len); 
	assert(node->value);
	memcpy(node->value, value, len);

	node->node_len        = len;
	node->next            = NULL;

	if (list->len == 0) {
		list->head = node;
		node->next = NULL;
	} else {
		p = list->head;
		while (p->next != NULL) {
			p = p->next;
		}
		p->next    = node;
		node->next = NULL;
	}
	
	list->len++;
	list->text_count = list->text_count + sizeof(int) + len;
}

static void listPrint(struct list *list)
{
	printf("node->print:\n");
	struct listNode *node = list->head;
	while (node != NULL) {
		printf("\t\t");
		printf("node:%d value:%s\n", node->node_len, node->value);
		node = node->next;
	}
}

static void listDestroy(struct list *list)
{
	struct listNode *node = list->head;

	if (list == NULL) {
		return;
	}

	if (node == NULL) {
		free(list);
		list = NULL;
		return;
	}

	while (node != NULL) {
		struct listNode *tmp = node;
		node = node->next;
		free(tmp->value);
		free(tmp);
	}

	free(list);
	list = NULL;
}
