#include "../comm_list.h"

struct list_test {
	int	a ;
	struct comm_list list;
};

void destroy(void *usr)
{
	struct list_test *test = (struct list_test*)usr;
	printf("destroy id:%d\n",test->a);
	return ;
}


int main()
{
	int i = 0;
	struct comm_list head = {};
	struct list_test list[10] = {};

	commlist_init(&head, destroy);

	for (i = 0; i < 10; i++) {
		list[i].a = i+1;
		if ( i <  6) {
			/* 只往链表中存6个节点 */
			commlist_push(&head, &list[i].list);
		}
	}

	struct comm_list *data = NULL;
	struct list_test *container = NULL;
	int offset = get_member_offset(&list[0], &list[0].list);

	for (i = 0; i < 10; i++) {
		/* 获取链表节点中的数据而不删除链表节点 */
		if (commlist_get(&head, &data)) {
			container = (struct list_test*)get_container_addr(data,offset);
			printf("get node id:%d\n", container->a);
		} else {
			printf("done get node\n");
			break;
		}
	}
	printf("\n");

	/* 删除指定的节点 */
	printf("delete node:%d\n", list[0].a);
	printf("delete node:%d\n", list[4].a);
	printf("delete node:%d\n\n", list[5].a);
	commlist_delete(&head, &list[0].list);
	commlist_delete(&head, &list[4].list);
	commlist_delete(&head, &list[5].list);

	/* 再往链表中插入一个数据 */
	commlist_push(&head, &list[6].list);
	printf("push node:%d\n\n", list[6].a);

	for (i = 0; i < 3; i++) {
		/* 获取链表节点中的数据并删除此节点 */
		if (commlist_pull(&head, &data)) {
			container = (struct list_test*)get_container_addr(data, offset);
			printf("pull id:%d \n", container->a);
		} else {
			break ;
		}
	}
	commlist_destroy(&head, offset);
	return 0;
}
