CC = gcc
AR = ar

CFLAG = -O2 -W -Wall -std=c99

INCLUDE_PATH = -I. -I../ -I/usr/include/ \
	-I../../../lib/libcomm \
	-I../../../lib/libcomm/mfptp_protocol \
	-I../../../lib/utils  \
	-I../../../lib/libkv/src \
	-I../../../lib/liblog \
#	-I../../../lib/json-c 
LIBA_PATH = -L../../../lib/libkv/src \
	-L../../../lib/ \
	-L/usr/local/lib/ \
	-L /root/work/supex/lib/liblog/slog/lib/
LIBA = -lm -ldl -lpthread -luuid  -lcomm -lutils -lkv -ljson-c -lslog 
TARGET = mul_client
#OBJ = client_simulation.o test_main.o
SRC = client.c
#SRC = client.c

all: $(TARGET)


$(TARGET) :
	$(CC) $(SRC) $(INCLUDE_PATH) $(LIBA_PATH) $(LIBA) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
