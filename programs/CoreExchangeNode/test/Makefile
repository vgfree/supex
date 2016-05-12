CC = gcc
AR = ar

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../ -I../../../lib/utils
TARGET = test
OBJ = gateway_simulation.o client_simulation.o test_main.o
SRC = gateway_simulation.c client_simulation.c test_main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -lm -ldl ../../../lib/utils/lib/libutils.a -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
