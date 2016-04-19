CC = gcc
AR = ar
TARGET = CoreExchangeNode

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../../lib/libcomm -I../../log_system/slog/
TARGET = ./core_exchange_node
OBJ = router.o config_reader.o daemon.o fd_manager.o message_dispatch.o main.o
SRC = router.c config_reader.c daemon.c fd_manager.c message_dispatch.c main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -L. -llog4cxx -lslog -lcomm -lstdc++ -lpthread -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
