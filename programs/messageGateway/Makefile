BASE_SRC=./http_dispatch/src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser -I../../lib/libscco -I../../engine -I../../engine/major -I./src -I./http_dispatch/src/ -I../../lib/libcomm/ -I../../lib/utils -I../../lib/zeromq-4.1.4/include/ -I../../lib/liblog -I../../lib/libmini/include -I../../lib/libevcoro/include
LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/  -L../../lib/liblog/slog/lib -llog4cxx -lslog -lstdc++

LIBS=-lluajit-5.1 -lpthread -lrt -ldl -lm
LIBA=-ljson-c

INC_LIBS=../../lib/utils/lib/libutils.a ../../lib/libcomm/lib/libcomm.a ../../lib/libzmq.a

#[=========>MARK<=========]#
MAIN_APP_SERV ?= messageGateway

HANDLE_MODEL = LINE
ADD_OBJ += swift_lua_api.o http_dispatch.o

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#



BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   load_swift_cfg.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CFLAGS += -g
CLEANFILES = mhooktrace.log

MAIN_OBJ = comm_io_wraper.c \
	   zmq_io_wraper.c \
	   zmq_pull_thread.c \
	   upstream.c \
	   validate_logon.c \
	   downstream.c \
	   message_concentrator.c \
	   main.c

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) $(MAIN_OBJ) $(INC_LIBS) -o $@
#	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf bin_messageGateway
	rm messageGateway
	rm -f $(CLEANFILES)
	rm -rf logs

distclean:
	make clean
	rm -rf ./logs/*.log
	
