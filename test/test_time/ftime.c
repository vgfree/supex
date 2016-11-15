#include <stdio.h>
#include <sys/timeb.h>

long long get_system_time(void) {
    struct timeb t;
    ftime(&t);
    return 1000 * t.time + t.millitm;
}

int main() {
    long long start=get_system_time();
    sleep(3);
    long long end=get_system_time();

    printf("time: %lld ms\n", end-start);
    return 0;
}
