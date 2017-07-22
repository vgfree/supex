CC = gcc
CFLAGS += -g -std=gnu99 -Wall -Woverride-init -Wsign-compare -Wtype-limits -Wuninitialized

.PHONY: test

all: rsocket.o

test: rsocket.c rsocket.h test_rsocket.c
	$(CC) $(CFLAGS) test_rsocket.c -o test_rsocket
	./test_rsocket
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--leak-resolution=high \
		--num-callers=20 \
		./test_rsocket

clean:
	rm -f rsocket.o
	rm -f test_rsocket

rsocket.o: rsocket.c rsocket.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
