1: Makefile connect_srv.c lazy_recv.c
	gcc -g -c connect_srv.c
	gcc -g -std=c99 -o lazy_recv lazy_recv.c connect_srv.o
	./lazy_recv -s127.0.0.1 -p9091 -t30 -b1

2: http_pressure_test.c Makefile connect_srv.c
	gcc -g -c connect_srv.c
	gcc -g -std=c99 -o a.out http_pressure_test.c connect_srv.o -lpthread
	./a.out -s127.0.0.1 -p9091 -c200000 -t8
clean:
	rm *.out *.o
