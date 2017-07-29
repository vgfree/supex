#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "comm_slist.h"

/***************************************************
 * commslist_create receives function pointers to cf and df, then it allocates enough space on the heap for a struct of comm_slist, !!warning!! sizeof(struct comm_slist *)
 *   does not provide sufficient space then you set all of the allocates memory to 0 so we know that everything points to * "empty"
 **************************************************/
void commslist_init(struct comm_slist *slist, SLIST_COMPARE_FUNC cf, SLIST_DESTRUCT_FUNC df)
{
	memset(slist, 0, sizeof(struct comm_slist));
	slist->cmp_fcb = cf;
	slist->del_fcb = df;
}

/***************************************************
 *   traverse and free.
 **************************************************/
void commslist_destroy(struct comm_slist *slist)
{
	struct comm_snode       *curr = slist->head;

	while (curr != NULL) {
		struct comm_snode       *del = curr;
		curr = curr->next;

		if (slist->del_fcb) {
			slist->del_fcb(del->data);
		}
		free(del);
	}
}

/***************************************************
 * Helper function for commslist_insert, allocates space on heap for struct comm_snode* and initializes values. n1->data points to newObj, next points to 0 or NULL,
 * and the amount of pointers pointing to this node is set to 1.
 **************************************************/
static struct comm_snode *create_node(void *newObj, uint64_t score)
{
	struct comm_snode *sn = malloc(sizeof(struct comm_snode));

	sn->data = newObj;
	sn->next = NULL;
	sn->score = score;
	return sn;
}
static void destory_node(struct comm_snode *sn)
{
	free(sn);
}

static int default_compare(void *v1, void *v2)
{
	uint64_t score1 = (uint64_t)v1;
	uint64_t score2 = (uint64_t)v2;
	if (score1 == score2) {
		return 0;
	}
	return score1 > score2 ? 1 : -1;
}


/***************************************************
 * commslist_insert is simple linked list insertion, it takes a list ptr, and void pointer to newObj. struct comm_snode* tmp points to the node created in create_node helper function.
 * Four cases are covered in this function which are: head ==  NULL, if new node > head, new node belongs somewhere in the middle of the list, and new node
 * belongs at the tail. Also, if something is a duplicate then it will immediately be freed upon detection.
 **************************************************/
int commslist_insert(struct comm_slist *slist, void *newObj, uint64_t score)
{
	struct comm_snode *tmp = create_node(newObj, score);

	if (slist->head == NULL) {
		slist->head = tmp;
		return 1;
	}

	int comp = 0;
	// if new node is bigger than head
	if (slist->cmp_fcb) {
		comp = slist->cmp_fcb(slist->head->data, tmp->data);
	} else {
		comp = default_compare((void *)slist->head->score, (void *)tmp->score);
	}
	if (comp <= 0) {
		tmp->next = slist->head;
		slist->head = tmp;
		return 1;
	}

	// if node is somewhere other than first or null
	struct comm_snode       *curr = slist->head->next;
	struct comm_snode       *prev = slist->head;

	while (curr != NULL) {
		if (slist->cmp_fcb) {
			comp = slist->cmp_fcb(curr->data, tmp->data);
		} else {
			comp = default_compare((void *)curr->score, (void *)tmp->score);
		}
		if (comp <= 0) {
			tmp->next = curr;
			prev->next = tmp;
			return 1;
		} else {
			prev = curr;
			curr = curr->next;
		}
	}

	// inserts at tail
	prev->next = tmp;
	return 1;
}

/***************************************************
 * SLRemove takes list and void pointer to data, create struct comm_snode* del to keep access to obj being removed. First case is if head is target and then if target is
 * somewhere else. If pointCount == 0 then that means no iterator is pointing to it so we can remove and free memory immediately otherwise if it does not == 0
 * then we simply remove it from list.
 **************************************************/
int commslist_remove(struct comm_slist *slist, void **oldObj, uint64_t *score)
{
	if (slist->head == NULL) {
		*oldObj = NULL;
		*score = 0;
		return 0;
	} else {
		struct comm_snode *sn = slist->head;
		slist->head = sn->next;

		*oldObj = sn->data;
		*score = sn->score;
		destory_node(sn);
		return 1;
	}
}

