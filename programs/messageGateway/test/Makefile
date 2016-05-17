INC =  -I../../../lib/zeromq-4.1.4/include/
LIB =  -L../../../lib -L/usr/local/lib
TARGET = AppSrvTest

$(TARGET):
	gcc -g $(OPT) $(INC) $(LIB) -lpthread -lrt login_server.c message_server.c UserInfoApi.c main.c -lzmq  -lstdc++  -o $@
clean:
	rm -rf $(TARGET)

