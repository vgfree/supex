
INC =  -I../../lib/zeromq-4.2.1/include/ -I/usr/local/include/luajit-2.0 -I./
LIB =  -L../../lib -L/usr/local/lib

OPT = -D_REENTRANT -D_GNU_SOURCE -DDEBUG -DBASE_THREADSAFE \
      -DSELECT_MULTIPROCESS


all:
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt  skt.c main.c -lzmq  -lstdc++  -o transit

clean:
	rm -rf transit


push:
	git push origin HEAD:refs/for/master
