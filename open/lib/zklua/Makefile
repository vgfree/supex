all: zklua.so

# You may need to change the following variables according to
# your system, here are the descriptions of these variables:
#
# ZOOKEEPER_LIB_DIR: Zookeeper C API library path.
# LUA_LIB_DIR: Lua library path.
# LUA_VERSION: Lua version.
# LUA_VERSION_NUMBER: Lua version number.
LIB_DIR = ./lib
ZOOKEEPER_LIB_DIR = $(LIB_DIR)
ZOOKEEPER_INC_DIR = $(LIB_DIR)/build/include
LUA_LIB_DIR = /usr/local/lib/lua
LUA_INC_DIR = /usr/local/include/luajit-2.0
LUA_VERSION = lua
LUA_VERSION_NUMBER = 5.1

CC = gcc
CFLAGS = -I$(ZOOKEEPER_INC_DIR) -I$(LUA_INC_DIR) -fPIC -O2 -Wall
INSTALL_PATH = $(shell pkg-config $(LUA_VERSION) --variable=libdir)/$(LUA_VERSION)/$(LUA_VERSION_NUMBER)

OS_NAME = $(shell uname -s)
MH_NAME = $(shell uname -m)

ifeq ($(OS_NAME), Darwin)
LDFLAGS += -bundle -undefined dynamic_lookup -framework CoreServices
ifeq ($(MH_NAME), x86_64)
endif
else
LDFLAGS += -shared -lrt
endif

#LDFLAGS += -lm -ldl -lpthread -L$(ZOOKEEPER_LIB_DIR) -lzookeeper_mt
LDFLAGS += -lm -ldl -lpthread -L$(ZOOKEEPER_LIB_DIR) -lzookeeper_mt

SRCS := zklua.c

OBJS := $(patsubst %.c,%.o,$(SRCS))

$(OBJS):
	$(CC) -c $(CFLAGS) $(SRCS)

libs:
	$(MAKE) -C $(LIB_DIR)

zklua.so: libs $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)


.PHONY: all clean install

clean:
	rm -f *.o *.so
	$(MAKE) -C $(LIB_DIR) clean

install: zklua.so
ifeq ($(OS_NAME), Darwin)
	install zklua.so $(INSTALL_PATH)/zklua.so
else
	install -D -s zklua.so $(INSTALL_PATH)/zklua.so
endif
