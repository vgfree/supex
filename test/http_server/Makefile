PREDEFINE=-DLOG_DEBUG=1 -DHTTP_OK=1 -DBENCHMARK=0 -DMAX_RECV_BUF=4096
CFLAGS=-pipe -std=gnu99 -W -Wall -Wpointer-arith -Wno-unused-parameter \
	   -Werror -g -ggdb3 $(PREDEFINE) \
	   -pg -fprofile-arcs -ftest-coverage

ALL_INCS = -I ./include

ALL_DEPS = http/http.h http/client.h http/worker.h http/http_parser.h

http: http.o worker.o client.o http_parser.o main.o
	gcc $(CFLAGS) -o srv \
		http.o worker.o client.o http_parser.o main.o \
		-L/usr/local/lib -lpthread -levent 

run:
	./srv -s0.0.0.0 -p9091 -w4

valgrind:
	valgrind --tool=memcheck --leak-check=full ./srv -s192.168.1.101 -p1314 -w4

push:
	git push origin HEAD:refs/for/master

main.o : $(ALL_DEPS) \
	main.c
	gcc -c $(CFLAGS) $(ALL_INCS) \
		-o main.o main.c

http.o : $(ALL_DEPS) \
	http/http.c
	gcc -c $(CFLAGS) $(ALL_INCS) \
		-o http.o http/http.c

worker.o : $(ALL_DEPS) \
	http/worker.c
	gcc -c $(CFLAGS) $(ALL_INCS) \
		-o worker.o http/worker.c

client.o : $(ALL_DEPS) \
	http/client.c
	gcc -c $(CFLAGS) $(ALL_INCS) \
		-o client.o http/client.c

http_parser.o : $(ALL_DEPS) \
	http/http_parser.c
	gcc -c $(CFLAGS) $(ALL_INCS) \
		-o http_parser.o http/http_parser.c

cscope:
	find . -name "*.h" -o -name "*.c" > cscope.files
	cscope -bkq -i cscope.files
	ctags --languages=+c --recurse=yes *

clean:
	- rm -rf *.o *.gcda *.gcno *.gcov cscope.* *.dSYM *.out 
