
SRV=pole-M
MAIN_APP_SERV ?= $(SRV)

LOG=./logs
SRC=./src
BIN=./bin

INC_PATH=\
	 -I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c \
	 -I../../lib/libkv/release \
	 -I../../lib/libmemhook/src \
	 -I../../lib/libscco/include \
	 -I../../lib/libcoro/include \
	 -I../../lib/libevcoro/include \
	 -I../../lib/libevcs/include/engine \
	 -I../../lib/libevcs/include/frame \
	 -I./src -I../ \
	 -I../../lib/leveldb-1.18/include/ \
	 -I../../lib/zeromq-4.1.4/include/ \
	 -I../../lib/libmini/include/ \
	 -I../../lib/libxmq/include/ \
	 -I../../lib/libnetmod/include/

LIB_PATH=-L../../lib

LIBS=-lpthread -lrt -lm -ldl -lluajit-5.1 -lstdc++
LIBA=-lleveldb -ljson-c -lmini -levcoro -lxmq -lnetmod -lzmq -lsnappy

#<------------------------>#
HANDLE_MODEL = EVCORO
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

# [args] Running mode, DEBUG: debug mode. RELEASE: release mode.
ifeq ($(COMPILE), RELEASE)
  CFLAGS += -O2 -DNDEBUG -Wall
else
  CFLAGS += -g -O0 -DDEBUG
endif

ifeq ($(TCLEVEL), THREAD)
  DEP_OBJS= pipe.o porter.o switcher.o
  CFLAGS += -DTC_THREAD
else
  DEP_OBJS= event_dispenser.o event_pipe.o event_handler.o
  CFLAGS += -DTC_COROUTINE
endif

OBJS=$(addprefix $(BIN)/, \
	$(DEP_OBJS) \
	ldb_cb.o \
	log.o \
	conf.o \
	load_swift_cfg.o \
	swift_cpp_api.o \
	main.o \
	)

JOB ?= $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJS)
	@-if [ ! -d $(LOG) ];then mkdir $(LOG); fi
	gcc $(CFLAGS) $^ $(LIB_PATH) $(LIBA) $(LIBS) -o $@

$(BIN)/%.o : $(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_PATH) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(SRV)

DEL:
	rm -rf $(LOG)/*.log nohup.out
	rm -rf ./data/* ./kvstate

distclean:
	make clean
	make DEL
