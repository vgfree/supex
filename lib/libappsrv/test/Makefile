CC = gcc
AR = ar

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../ -I../../zeromq-4.1.4/include/
TARGET = test
OBJ = main.o
SRC = main.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -lm -ldl -lpthread ../lib/libappsrv.a ../../libzmq.a ../../utils/lib/libutils.a ../../libmini.a -lstdc++ -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
