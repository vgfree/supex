CC = gcc
AR = ar

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../ -I../../../lib/libcomm -I../../../lib/utils  -I../../../lib/libkv/src
TARGET = test
OBJ = client_simulation.o test_main.o
SRC = client_simulation.c test_main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -lm -ldl -lpthread ../../../lib/libcomm/lib/libcomm.a ../../../lib/utils/lib/libutils.a ../../../lib/libkv/src/libkv.a -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
