/*
 *  binheap.h
 *
 *  Created on: Jan 27, 2016
 *  Author: shu
 * */

#ifndef __BinHeap_H
#define __BinHeap_H

#include "topo_com.h"

#define MinPQSize       (10)
#define MinData         (-32767)

#define UpdateItem      int
typedef struct vehicle_obj *ElementType;

struct HeapStruct
{
	int             Capacity;
	int             Size;
	int             lazy;
	int             count;
	ElementType     *Elements;
};

typedef struct HeapStruct *PriorityQueue;

PriorityQueue Initialize(int MaxElements);

void Destroy(PriorityQueue H);

void MakeEmpty(PriorityQueue H);

void Insert(ElementType X, PriorityQueue H);

ElementType DeleteMin(PriorityQueue H);

ElementType FindMin(PriorityQueue H);

int IsEmpty(PriorityQueue H);

int IsFull(PriorityQueue H);

void Update(PriorityQueue H, UpdateItem X);
#endif	/* ifndef __BinHeap_H */

