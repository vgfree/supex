CC = gcc
AR = ar
TARGET = CoreExchangeNode

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../../lib/libcomm -I../../lib/liblog -I../../lib/utils -I../../lib/libkv/src
TARGET = ./core_exchange_node
OBJ = fd_manager.o message_dispatch.o uid_map.o gid_map.o main.o
SRC = fd_manager.c message_dispatch.c uid_map.c gid_map.c main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -L../../lib/liblog/slog/lib -llog4cxx -lslog ../../lib/libcomm/lib/libcomm.a ../../lib/utils/lib/libutils.a -lstdc++ -lpthread ../../lib/libkv/src/libkv.a -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
