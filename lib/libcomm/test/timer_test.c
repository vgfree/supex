#include "../comm_timer.h"

void timer_event(void *usr)
{
	int *temp = (int*)usr;
	printf("\ndeeal with timer id :%d\n\n", *temp);
}


int main()
{
	int i = 0;
	int temp[5] = {0};
	struct comm_list timerhead = {};
	struct comm_timer *commtimer[5] = {NULL};

	commlist_init(&timerhead, NULL);

	for (i = 0; i < 5; i++) {
		temp[i] = i+1;
		commtimer[i] = commtimer_create(5100, 0, timer_event, (void*)&temp[i]);
		if (commtimer[i] == NULL) {
			int j = 0;
			printf("create timer failed\n");
			for (j = 0; j < i; j++) {
				commtimer_destroy(commtimer[j]);
				return -1;
			}
		} else {
			printf("create timer successed\n");
		}
		if (i == 3 || i == 1) {
			/* 这两个计时器间隔触发 */
			commtimer[i] = commtimer_create(5100, 3100, timer_event, (void*)&temp[i]);
		}
	}
	printf("\n");

	for (i = 0; i < 5; i++) {
		/* 启动计时器 */
		commtimer_start(commtimer[i], &timerhead);
	}

	for (i = 0; i < 100; i++) {
		commtimer_schedule(&timerhead);
		printf("test timer counter: %d\n", i);
		sleep(1);
	}

	for (i = 0; i < 5; i++) {
		commtimer_stop(commtimer[i], &timerhead);
		commtimer_destroy(commtimer[i]);
	}

	return 0;
}
