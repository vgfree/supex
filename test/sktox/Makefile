OPT = -D_REENTRANT -D_GNU_SOURCE -DDEBUG -DBASE_THREADSAFE \
      -DSELECT_MULTIPROCESS


LIBRARY_SELECT = ZMQ

ifeq ($(LIBRARY_SELECT), ZMQ)
OPT += -DSELECT_ZMQ
else
OPT += -DSELECT_CZMQ
endif

all:
ifeq ($(LIBRARY_SELECT), ZMQ)
	gcc -g $(OPT) -lzmq skt.c x.c -o x.exe
	gcc -g $(OPT) -lzmq skt.c y.c -o y.exe
	gcc -g $(OPT) -lzmq skt.c z.c -o z.exe
else
	gcc -g $(OPT) -lczmq -lzmq cskt.c x.c -o x.exe
	gcc -g $(OPT) -lczmq -lzmq cskt.c y.c -o y.exe
	gcc -g $(OPT) -lczmq -lzmq cskt.c z.c -o z.exe
endif

clean:
	rm -rf *.exe


push:
	git push origin HEAD:refs/for/master
