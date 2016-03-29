/*
 * binheap->c
 *
 *  Created on: Jan 27, 2016
 *  Author: shu
 */

#include "fatal.h"
#include "binheap.h"

PriorityQueue Initialize(int MaxElements)
{
	PriorityQueue H;

	if (MaxElements < MinPQSize) {
		Error("Priority queue size is too small!");
	}

	H = malloc(sizeof(struct HeapStruct));

	if (H == NULL) {
		FatalError("Out of space!!!");
	}

	/* Allocate the array plus one extra for sentinel */
	H->Elements = malloc((MaxElements + 1)
			* sizeof(ElementType));

	if (H->Elements == NULL) {
		free(H);
		FatalError("Out of space!!!");
	}

	H->Capacity = MaxElements;
	H->Size = 0;
        H->lazy = 0;
        H->count= 0;
	H->Elements[0]->priority = MinData;

	return H;
}

/* H->Elements[ 0 ] is a sentinel */
void Insert(ElementType X, PriorityQueue H)
{
	int i;

	if (IsFull(H)) {
		Error("Priority queue is full");
		return;
	}

	for (i = ++H->Size; H->Elements[i / 2]->priority > X->priority; i /= 2) {	/* The new element is percolated up the heap  */
		H->Elements[i] = H->Elements[i / 2];		/* until the correct location is found */
	}

	H->Elements[i] = X;
}

ElementType DeleteMin(PriorityQueue H)
{
	int             i, Child;
	ElementType     MinElement, LastElement;

	if (IsEmpty(H)) {
		Error("Priority queue is empty!");
		return H->Elements[0];
	}

	MinElement = H->Elements[1];
	LastElement = H->Elements[H->Size--];

	for (i = 1; i * 2 <= H->Size; i = Child) {
		/* Find smaller child */
		Child = i * 2;

		if ((Child != H->Size) && (H->Elements[Child + 1]->priority < H->Elements[Child]->priority)) {
			Child++;
		}

		/* Percolate one level */
		if (LastElement->priority > H->Elements[Child]->priority) {
			H->Elements[i] = H->Elements[Child];
		} else {
			break;
		}
	}

	H->Elements[i] = LastElement;
	return MinElement;
}

void MakeEmpty(PriorityQueue H)
{
	H->Size = 0;
}

ElementType FindMin(PriorityQueue H)
{
	if (!IsEmpty(H)) {
		return H->Elements[1];
	}

	Error("Priority Queue is Empty");
	return H->Elements[0];
}

int IsEmpty(PriorityQueue H)
{
	return H->Size == 0;
}

int IsFull(PriorityQueue H)
{
	return H->Size == H->Capacity;
}

void Destroy(PriorityQueue H)
{
	if (H->Elements) {
		free(H->Elements);
	}

	if (H) {
		free(H);
	}
}

void Update(PriorityQueue H, UpdateItem X)
{
        int num = 0;

        if( IsEmpty( H ) ) {
            Error("PriorityQueue is Empty");
            return;
        }
        
        if( X <=0 ) {
                printf("X <= 0 \n");
                return;
        }

        num = H->Size;
        while(num > 0) {
                printf("## id:%d priority:%d %d\n", H->Elements[num]->roadid, H->Elements[num]->priority, X);
                H->Elements[num]->priority -= X;
                if(H->Elements[num]->priority == 0)
                        H->count++;

                num--;
        }
}
