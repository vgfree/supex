CC = gcc
AR = ar
TARGET = CoreExchangeNode

CFLAG = -g -O1 -W -Wall

INCLUDE_PATH = -I. \
	       -I/usr/include \
	       -I/usr/local/include \
	       -I$(HOME_PATH)/lib/libcomm \
	       -I$(HOME_PATH)/lib/iniparser/src \
	       -I$(HOME_PATH)/lib/libmini/include \
	       -I$(HOME_PATH)/lib/libkv/release/
TARGET = ./core_exchange_node
OBJ = fd_manager.o message_dispatch.o status.o uid_map.o gid_map.o notify.o main.o
SRC = fd_manager.c message_dispatch.c status.c uid_map.c gid_map.c notify.c main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -L$(HOME_PATH)/lib -L../ -lcomm -liniparser -lpthread -luuid -lkv -lleveldb -lstdc++ -lmini -ldl -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

distclean:
	make clean
