INC =  -I../../../lib/zeromq-4.1.4/include/ -I/usr/local/include/luajit-2.0 -I./
LIB =  -L../../../lib -L/usr/local/lib

all:
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt zmq_client.c -lzmq  -lstdc++  -o zmqclient
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt zmq_realtest.c -lzmq -lstdc++  -o zmqrealtest
clean:
	rm -rf zmqclient zmqrealtest

