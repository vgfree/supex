#include "timer.h"
#include "loger.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define TIMER_CAP 10

static uint64_t get_tick_count()
{
  struct timeval tval;
  uint64_t ret_tick;
  gettimeofday(&tval, NULL);
  ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
  return ret_tick;
}


struct timer_list *create_timer_list()
{
  struct timer_list *list =
    (struct timer_list*)malloc(sizeof(struct timer_list));
  assert(list);
  list->cap = TIMER_CAP;
  list->max_pos = -1;
  list->timer =
    (struct timer_item *)calloc(list->cap, sizeof(struct timer_item));
  assert(list->timer);
  memset(list->timer, 0, list->cap * sizeof(struct timer_item));
  return list;
}

int destroy_timer_list(struct timer_list *list)
{
  assert(list);
  if(list->timer) {
    free(list->timer);
  }
  free(list);
}

// 返回timer 编号.
int add_timer(struct timer_list *list, struct timer_item *timer)
{
  assert(list);
  assert(timer);
  assert(timer->callback);
  if (list->max_pos == TIMER_CAP - 1) {
    printf("Already is max timer numbers:%d.", list->max_pos);
    return -1;
  }
  timer->next_tick = get_tick_count() + timer->interval;
  for (int i = 0; i <= list->max_pos; i++) {
    if (list->timer[i].next_tick == 0) {
      list->timer[i] = *timer;
      return i;
    }
  }
  ++list->max_pos;
  list->timer[list->max_pos] = *timer;
  return list->max_pos;
}

int remove_timer(struct timer_list *list, uint8_t id)
{
  assert(list);
  assert(id < TIMER_CAP);
  if (list->max_pos == -1) {
    printf("no timer to delete.");
    return -1;
  }
  list->timer[id].callback = NULL;
  list->timer[id].user_data = NULL;
  list->timer[id].interval = 0;
  list->timer[id].next_tick = 0;
  if (id == list->max_pos) {
    --list->max_pos;
  }
  return id;
}

void schedule_timer(struct timer_list *list)
{
  log("schedule_timer");
  assert(list);
  uint64_t curr_tick = get_tick_count();
  log("max_pos:%d.", list->max_pos);
  for (int i = 0; i <= list->max_pos; i++) {
    struct timer_item *pitem = &list->timer[i];
    log("curr_tick:%ld, next_tick:%ld.", curr_tick, pitem->next_tick);
    if (curr_tick >= pitem->next_tick) {
      pitem->next_tick += pitem->interval;
      pitem->callback(pitem->user_data);
	}
  }
}
