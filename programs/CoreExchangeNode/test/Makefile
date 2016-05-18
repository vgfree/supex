CC = gcc
AR = ar

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../ -I../../../lib/libcomm -I../../../lib/utils -I../../../lib/liblog -I../../../lib/libkv/src
TARGET = test
OBJ = gid_map.o uid_map.o test_uid_map.o test_gid_map.o gateway_simulation.o client_simulation.o test_main.o
SRC = ../gid_map.c ../uid_map.c test_uid_map.c test_gid_map.c gateway_simulation.c client_simulation.c test_main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -lm -ldl -lpthread ../../../lib/libcomm/lib/libcomm.a ../../../lib/utils/lib/libutils.a ../../../lib/libkv/src/libkv.a -L../../../lib/liblog/slog/lib -llog4cxx -lslog -lstdc++ -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
