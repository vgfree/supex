INC =  -I../../../lib/zeromq-4.1.4/include/ -I/usr/local/include/luajit-2.0 -I./
LIB =  -L../../../lib -L/usr/local/lib

all:
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt zmq_test.c -lzmq  -lstdc++  -o zmqtest
	gcc -g $(OPT) $(INC) $(LIB)  -lluajit-5.1 -lpthread -lrt zmq_client.c -lzmq  -lstdc++  -o zmqClient
clean:
	rm -rf zmqtest zmqClient

