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

static void * _produce(void *varlist)
{
  struct free_queue_list *list = (struct free_queue_list *)varlist;
  while (1) {
    char var = (char) rand();
	if (free_queue_push(list, &var) == false) {
      printf("insert failed....");
	}
    printf("produce var:%02x\n", var);
    _print_list_info(list);
   sleep(1);
  }
  return NULL;
}

static void *_consumer(void *varlist)
{
  struct free_queue_list *list = (struct free_queue_list *)varlist;
  while (1) {
    char var;
    if (free_queue_pull(list, &var) == true) {
      printf("consumer var:%02x\n", var);
	}
    _print_list_info(list);
    sleep(6);
  }
  return NULL;
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
  srand(235);
  pthread_t tid1, tid2;
  pthread_create(&tid1, NULL, _produce, &list);
  pthread_create(&tid1, NULL, _consumer, &list);
  void *status;
  pthread_join(tid1, &status);
  pthread_join(tid2, &status);
}
