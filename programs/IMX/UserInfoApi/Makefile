INC =  -I$(HOME_PATH)/lib/zeromq-4.2.1/include/ \
       -I/usr/local/include/luajit-2.0 \
       -I./
LIB =  -L$(HOME_PATH)/lib -L/usr/local/lib

OPT = -D_REENTRANT -D_GNU_SOURCE -DDEBUG -DBASE_THREADSAFE \
      -DSELECT_MULTITHREAD

all:
	cd setting && gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt skt.c main.c -lzmq  -lstdc++ -lm  -o setting
	cd ..

clean:
	rm -rf setting/setting

distclean:
	make clean

push:
	git push origin HEAD:refs/for/master
