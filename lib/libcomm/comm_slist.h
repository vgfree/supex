#pragma once

/*
 * sort-list.h
 */

#include <stdlib.h>
#include <stdint.h>

// ======Prototypes for User-Defined Functions==========
// -=-=-=-You do not need to do anything with these definitions-=-=-=-

/*
 * Your list is used to store data items of an unknown type, which you need to sort.
 * Since the type is opaque to you, you do not know how to directly compare the data.
 *
 * You can presume though that a user will supply your code with a comparator function
 * that knows how to compare the data being sorted, but your code must do the rest
 * of the bookkeeping in a generic manner.
 *
 * The comparator function will take pointers to two data items and will return -1 if the 1st
 * is smaller, 0 if the two are equal, and 1 if the 2nd is smaller.
 *
 * The user will also supply a destruct function will take a pointer to a single data item
 *	and knows how to deallocate it. If the data item does not require deallocation, the user's
 *  destruct function will do nothing.
 *
 * Note that you are not expected to implement any comparator or destruct functions.
 *  Your code will have appropriate comparator function and a destruct functions added to it.
 */

typedef int (*SLIST_COMPARE_FUNC)(void *, void *);
typedef void (*SLIST_DESTRUCT_FUNC)(void *);

struct comm_snode
{
	void    *data;
	void    *next;
	uint64_t score;
};

// -=-=-=-You must implement all the functions and definitions below-=-=-=-

// =====0: comm_slist=====================================
// ===0.1: List Definition, List Create/Destroy

/*
 * Sorted list type that will hold all the data to be sorted.
 */
struct comm_slist
{
	struct comm_snode       *head;
	SLIST_COMPARE_FUNC      cmp_fcb;
	SLIST_DESTRUCT_FUNC     del_fcb;
};


/*
 * commslist_create creates a new, empty, 'comm_slist'.
 *
 * commslist_create's parameters will be a comparator (cf) and destructor (df) function.
 *   Both the comparator and destructor functions will be defined by the user as per
 *     the prototypes above.
 *   Both functions must be stored in the comm_slist struct.
 *
 * commslist_create must return NULL if it does not succeed, and a non-NULL struct comm_slist *
 *   on success.
 */

void commslist_init(struct comm_slist *slist, SLIST_COMPARE_FUNC cf, SLIST_DESTRUCT_FUNC df);

/*
 * commslist_destroy destroys a comm_slist, freeing all dynamically-allocated memory.
 */
void commslist_destroy(struct comm_slist *slist);



// ===0.2: List Insert/Remove

/*
 * commslist_insert inserts a given data item 'newObj' into a comm_slist while maintaining the
 *   order and uniqueness of list items.
 *
 * commslist_insert should return 1 if 'newObj' is not equal to any other items in the list and
 *   was successfully inserted.
 * commslist_insert should return 0 if 'newObj' is equal to an item already in the list or it was
 *   not successfully inserted
 *
 * Data item equality should be tested with the user's comparator function *
 */

int commslist_insert(struct comm_slist *slist, void *newObj, uint64_t score);

/*
 * SLRemove should remove 'newObj' from the comm_slist in a manner that
 *   maintains list order.
 *
 * SLRemove must not modify the data item pointed to by 'newObj'.
 *
 * SLRemove should return 1 on success, and 0 on failure.
 */

int commslist_remove(struct comm_slist *slist, void **oldObj, uint64_t *score);

