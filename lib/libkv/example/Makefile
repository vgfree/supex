# Makefile for example


LIBKV_LIB=../release/libkv.a -lpthread
LIBKV_INCLUDE=-I../release
TARGET_1 = example_basic
TARGET_2 = example_multi_handler
TARGET_3 = example_multi_thread
CFLAGS+=-Wall

all: $(TARGET_1) $(TARGET_2) $(TARGET_3)

$(TARGET_1):
	gcc -o $@ $(TARGET_1).c $(LIBKV_LIB) $(LIBKV_INCLUDE)

$(TARGET_2):
	gcc -o $@ $(TARGET_2).c $(LIBKV_LIB) $(LIBKV_INCLUDE)

$(TARGET_3):
	gcc -o $@ $(TARGET_3).c $(LIBKV_LIB) $(LIBKV_INCLUDE)

test:
	gcc -g -o $@ test.c $(LIBKV_LIB) $(LIBKV_INCLUDE)

clean:
	rm example test *.o *.*~ -f
	rm example_basic example_multi_handler example_multi_thread -f
