PWD = $(shell pwd)

INC_DIR = -I../../lib/hiredis -I../../lib/libev-4.15 -I../../lib/json-c -I../../lib/graphlabapi/src -I/usr/local/include/luajit-2.0

LIB_DIR = -L/usr/local/lib -L../../lib

LIBS = -lrt -lm -lhiredis -lpthread

LIBA = -lev -ljson-c -lluajit-5.1 -lboost_program_options -lgraphlab -lgraphlab_pic -fPIC

CFLAGS = -Wall -g -O0 \
	 -DOPEN_DEBUG

SRC_DIR = $(PWD)/src

SERVER_SRC = $(addprefix $(SRC_DIR)/, \
	     graph.cpp \
	     )
	     #log.c \

DEMO_SRC = $(addprefix $(SRC_DIR)/, \
	     graph_demo.cpp \
	     )

demo:
	if [ -f $(PWD)/demo ];then make clean; fi
	g++ $(CFLAGS) $(DEMO_SRC) $(INC_DIR) $(LIB_DIR) $(LIBS) $(LIBA) -o demo

RR:
	if [ -f $(PWD)/RR ];then make clean; fi
	g++ $(CFLAGS) $(SERVER_SRC) $(INC_DIR) $(LIB_DIR) $(LIBS) $(LIBA) -o RR

#all: demo RR
all: demo

clean:
	rm -rf $(PWD)/RR
	rm -rf $(PWD)/demo

distclean:
	if [ -f lib/Makefile ]; then cd $(PWD)/lib && make clean; fi
	rm -rf $(PWD)/RR
	rm -rf $(PWD)/demo
