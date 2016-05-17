INC =  -I../../../lib/zeromq-4.1.4/include/
LIB =  -L../../../lib -L/usr/local/lib
TARGET = test

$(TARGET):
	gcc -g $(OPT) $(INC) $(LIB) -lpthread -lrt zmq_sub.c -lzmq  -lstdc++  -o $@
clean:
	rm -rf $(TARGET)

