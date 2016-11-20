libsocket-server.a : socket_server.o
	ar -rcs $@ $^
	gcc -g -Wall -o test test.c $@ -lpthread

%.o:%.c
	gcc -g -Wall -c $<

clean:
	rm -f libsocket-server.a test *.o


