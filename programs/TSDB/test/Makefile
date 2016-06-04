
INC =  -I/usr/local/include/luajit-2.0 -I./
LIB =  -L../../../lib -L/usr/local/lib

OPT = -D_REENTRANT -D_GNU_SOURCE -DDEBUG -DBASE_THREADSAFE \
      -DSELECT_MULTIPROCESS


all:
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt main.c -lstdc++  -o test

clean:
	rm -rf test
