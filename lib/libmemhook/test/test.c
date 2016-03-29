#include <pthread.h>
#include "malloc_hook.h"
#include "node_manager.h"
#include "common_test.h"

/*
 *
 */
typedef struct
{
	size_t  len;
	char    buff[1];
} *StringT;

#define         TestThreads (4)

AO_T testflag = 0;

StringT NewString(long len);

void FreeString(StringT string);

void *TestMalloc(void *data);

#define         TESTTOTAL (200000)

int main(int argc, char **argv)
{
	pthread_t       tid[4] = {};
	int             i = 0;
	StringT         ptr = NULL;

	ptr = NewString(20);

	for (i = 0; i < DIM(tid); ++i) {
		/* code */
		pthread_create(&tid[i], NULL, TestMalloc, (void *)TESTTOTAL);
	}

	while (ATOMIC_GET(&testflag) < TestThreads) {
		sched_yield();
	}

	ManagerPrint("./log.out", 0);

	ATOMIC_SET(&testflag, 0);

	for (i = 0; i < DIM(tid); ++i) {
		/* code */
		pthread_join(tid[i], NULL);
	}

	FreeString(ptr);
	// sleep(20);

	PrintResult("TestMalloc");

	return EXIT_SUCCESS;
}

void *TestMalloc(void *data)
{
	long    count = (long)data;
	long    i = 0;

	char **ptr = NULL;

	BindCPUCore(-1);

	NewArray(count, ptr);

	for (i = 0; i < count; i++) {
		StartStat();
		NewArray(1, ptr[i]);
		EndStat("NewArray()");
	}

	ATOMIC_INC(&testflag);

	while (ATOMIC_GET(&testflag) != 0) {
		sched_yield();
	}

	for (i = 0; i < count; i++) {
		StartStat();
		Free(ptr[i]);
		EndStat("NewArray()");
	}

	return NULL;
}

StringT NewString(long len)
{
	StringT string = NULL;
	long    total = 0;

	total = (long)(&((StringT)0)->buff) + len;
	assert(total > (long)(&((StringT)0)->buff));

	string = malloc(total + 1);
	string->len = len;

	return string;
}

void FreeString(StringT string)
{
	Free(string);
}

