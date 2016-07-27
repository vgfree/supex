CC = gcc
AR = ar
TARGET = CoreExchangeNode

CFLAG = -g -O1 -W -Wall

INCLUDE_PATH = -I. -I../../lib/libcomm -I../../lib/utils -I../../lib/libkv/src -I../../lib/libmini/include -I/usr/include -I/usr/local/include
TARGET = ./core_exchange_node
OBJ = fd_manager.o message_dispatch.o status.o uid_map.o gid_map.o notify.o main.o
SRC = fd_manager.c message_dispatch.c status.c uid_map.c gid_map.c notify.c main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -L../../lib ../../lib/libcomm/lib/libcomm.a ../../lib/utils/lib/libutils.a -lpthread -lkv -lmini -ldl -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

distclean:
	make clean
