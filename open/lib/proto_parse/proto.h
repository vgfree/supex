#ifndef _PROTO_H_
#define _PROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define size 64

typedef struct listNode
{
	int             node_len;
	char            *value;
	struct listNode *next;
} node;

typedef struct list
{
	struct listNode *head;
	int             text_count;
	int             len;
} list;

struct list     *proto_init();

void proto_append(struct list *list, const char *data, int len);

char *proto_pack(struct list *list);

void proto_parse(struct list *list, const char *t);

void proto_get(struct list *list);

void proto_destroy(struct list *list);

void strfree(char *str);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef _PROTO_H_ */

