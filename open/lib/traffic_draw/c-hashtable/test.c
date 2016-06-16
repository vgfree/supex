/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *
 *   filename : hashtest.c
 *   author   : shumenghui@mirrtalk.com
 *   date     : 2016-04-19
 *   info     :
 * ======================================================== */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hashtable.h"

typedef struct _node
{
	int     name;
	double  dir;
} Node;

int main()
{
	printf("\n<int,double>\n");
	HashTable       *ht = create_hashtable(100, int, double);
	double          t = 2000000000.0;
	hash_add(ht, 100, t);
	hash_add(ht, 20, 15.0);
	double v;
	hash_find(ht, 20, &v);
	printf("\t20  :%lf\n", v);
	hash_find(ht, 100, &v);
	printf("\t100 :%lf\n", v);
	printf("\tset 100's value  = 200.0\n");
	hash_add(ht, 100, 200.0);
	hash_find(ht, 100, &v);
	printf("\t100 :%lf\n", v);
	int     key;
	double  tvalue;
	printf("\n\t[itering...]\n");

	for (reset(ht); isnotend(ht); next(ht)) {
		key = nkey(ht);
		tvalue = *(double *)value(ht);
		printf("\tkey: %d, value:%lf\n", key, tvalue);
	}

	hash_free(ht);
	ht = NULL;

	printf("\n<char*,int>\n");
	HashTable *ht1 = create_hashtable(100, char *, int);
	hash_add(ht1, "song", 1000);
	hash_add(ht1, "abcd", 1235451254);
	int value;
	hash_find(ht1, "song", &value);
	printf("\tsong: %d\n", value);
	hash_find(ht1, "abcd", &value);
	printf("\tabcd: %d\n", value);
	char *t_key;
	printf("\n\t[itering...]\n");

	for (reset(ht1); isnotend(ht1); next(ht1)) {
		t_key = skey(ht1);
		value = *(int *)value(ht1);
		printf("\tkey: %s, value:%d\n", t_key, value);
	}

	free(ht1);
	ht1 = NULL;

	printf("\n<char*,double>\n");

	HashTable *ht2 = create_hashtable(100, char *, double);
	hash_add(ht2, "whatafuck", 12314524.1235);
	hash_add(ht2, "xiaoqiang", 123.123);
	double b;
	hash_find(ht2, "whatafuck", &b);
	printf("\twhatafuck: %lf\n", b);
	hash_find(ht2, "xiaoqiang", &b);
	printf("\txiaoqiang: %lf\n", b);

	if (hash_exists(ht2, "xiaoqiang") == EXISTS) {
		printf("\texists xiaoqiang\n");
	}

	printf("\thash element counts: %d\n", hash_num_elements(ht2));
	printf("\tdel xiaoqiang...\n");
	hash_del(ht2, "xiaoqiang");

	if (hash_exists(ht2, "xiaoqiang") == NOTEXISTS) {
		printf("\tnot exists xiaoqiang\n");
	} else if (hash_exists(ht2, "xiaoqiang") == EXISTS) {
		printf("\tstill exists xiaoqiang\n");
	}

	printf("\thash element counts: %d\n", hash_num_elements(ht2));
	hash_free(ht2);
	ht2 = NULL;

	printf("\n<char*,char*>\n");

	HashTable *ht3 = create_hashtable(100, char *, char *);
	hash_add(ht3, "fuck", "make love");
	hash_add(ht3, "like", "not love");
	char *v_tmp;
	hash_find(ht3, "fuck", &v_tmp);
	printf("\tfuck: %s\n", v_tmp);
	hash_find(ht3, "like", &v_tmp);
	printf("\tlike: %s\n", v_tmp);
	printf("\n\t[itering...]\n");

	for (reset(ht3); isnotend(ht3); next(ht3)) {
		t_key = skey(ht3);
		v_tmp = *(char **)value(ht3);
		printf("\tkey: %s, value:%s\n", t_key, v_tmp);
	}

	hash_free(ht3);
	ht3 = NULL;

	printf("\n<long,intptr_t>\n");
	HashTable       *ht4 = create_hashtable(100, long, intptr_t);
	Node            one = {
		2,
		2.2
	};

	hash_add(ht4, 100, (intptr_t)&one);
	hash_add(ht4, 20, 1512312312);
	intptr_t re;
	hash_find(ht4, 20, &re);
	printf("\t20  :%ld\n", re);
	hash_find(ht4, 100, &re);
	printf("\t100 :%ld\n", re);
	// intptr_t nam = (*(intptr_t *)re);
	Node *re_one = (Node *)re;
	printf("--node %d %lf\n", re_one->name, re_one->dir);
	printf("\tset 100's value  = 200\n");
	hash_add(ht4, 100, 200);
	hash_find(ht4, 100, &re);
	printf("\t100 :%ld\n", re);
	long            key1;
	intptr_t        ivalue;
	printf("\n\t[itering...]\n");

	for (reset(ht4); isnotend(ht4); next(ht4)) {
		key1 = nkey(ht4);
		ivalue = *(intptr_t *)value(ht4);
		printf("\tkey: %ld, value:%ld\n", key1, ivalue);
	}

	hash_free(ht4);
	ht4 = NULL;

	return 0;
}

