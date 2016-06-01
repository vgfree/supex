CC = gcc
AR = ar

CFLAG = -O2 -W -Wall

INCLUDE_PATH = -I. -I../ -I../../libmini/include
TARGET = test
OBJ = test.o
SRC = test.c

all: $(TARGET)
$(OBJ): $(SRC)
	$(CC) $(CFLAG) $(INCLUDE_PATH) -c $(SRC)

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -lm -ldl ../lib/libutils.a ../../libmini.a -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
