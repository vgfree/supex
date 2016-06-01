#include "free_queue_list.h"

static _print_list_info(struct free_queue_list *list)
{
  assert(list);
  printf("list, max:%u, all:%u, dsz:%u, head:%u, tail:%u\n",
		 list->max, list->all, list->dsz, list->head, list->tail);
}

static void _insert_queue(struct free_queue_list *list, int number)
{
  for (int i = 0; i < number; i++) {
    char var = (char) i;
    free_queue_push(list, &var);
  }
  printf("insert %d data after\n", number);

}
static void _pop_queue(struct free_queue_list *list, int number)
{
  printf("pop %d data:\n", number);
  for (int i = 0 ; i < number; i++) {
    char var;
    free_queue_pull(list, &var);
    printf("%x-", var);
  }
  printf("\n");
}
int main (int argc, char *argv[]) {
  struct free_queue_list list;
  free_queue_init(&list, 1, 256);
  printf("list init after\n");
  _print_list_info(&list);
  _insert_queue(&list, 8);
  _print_list_info(&list);
  _pop_queue(&list, 8);
  _print_list_info(&list);
  _insert_queue(&list, 27);
  _print_list_info(&list);
  _pop_queue(&list, 27);
  _print_list_info(&list);

}
