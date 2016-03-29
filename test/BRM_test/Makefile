TARGET = BRM_cln_test

BUILD_DIR =	$(shell pwd)
BUILD_CFLAGS = -Wall 
BUILD_LIBS = -lluajit-5.1 -lpthread -lm -lstdc++ -ldl
BUILD_INC_DIR = -I/usr/include -I/usr/local/include -I/usr/local/include/luajit-2.0
BUILD_LIB_DIR = -L/usr/local/lib -L/usr/lib/ -L/usr/libexec
BUILD_OPTIMIZE =


LIB_DIR = -L$(BUILD_DIR)/../../lib -L$(BUILD_DIR)/../../engine

INC_DIR = -I$(BUILD_DIR)/../../lib/libmini/include \
		  -I$(BUILD_DIR)/../../lib/libmini/src \
		  -I$(BUILD_DIR)/../../lib/http-parser \
		  -I$(BUILD_DIR)/../../lib/libev \
		  -I$(BUILD_DIR)/../../lib/libcoro \
		  -I$(BUILD_DIR)/../../lib/libevcoro/include \
		  -I$(BUILD_DIR)/../../engine					

ARL_LIBS = -lsupex_base -lmini -levcoro -lcoro -lev

CFLAGS = -g -D_GNU_SOURCE

CC_COMPILER ?=	gcc
CC = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_INC_DIR) $(INC_DIR) $(BUILD_OPTIMIZE)
LN = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_LIB_DIR) $(LIB_DIR)
AR =			@ar -rcs


OBJ_DIR = $(BUILD_DIR)/obj
OBJS = $(addprefix $(OBJ_DIR)/, \
	   	net.o \
		cache.o \
		BRM_cln_test.o \
		)
	   

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $< -o $@

all: prepare $(TARGET)

prepare:
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi

$(TARGET): $(OBJS)
	$(LN) $^ -o $@ $(ARL_LIBS) $(BUILD_LIBS)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(TARGET)
	rm -rf 20*
