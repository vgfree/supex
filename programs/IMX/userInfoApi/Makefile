INC =  -I$(HOME_PATH)/lib/zeromq-4.2.1/include/ \
       -I/usr/local/include/luajit-2.0 \
       -I./
LIB =  -L$(HOME_PATH)/lib -L/usr/local/lib

OPT = -DSELECT_MULTITHREAD

all:
	gcc -g $(OPT) main.c skt.c $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt -lzmq  -lstdc++ -lm  -o userInfoApi

clean:
	rm -f userInfoApi

distclean:
	make clean

push:
	git push origin HEAD:refs/for/master
