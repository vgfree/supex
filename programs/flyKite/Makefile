BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I/usr/local/include/ \
	-I../../lib/libev \
	-I../../lib/json-c \
	-I../../lib/libmini/include \
	-I../../lib/libkv/release \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco/include \
	-I../../lib/libcoro/include \
	-I../../lib/libevcoro/include \
	-I../../lib/libevcs/include/engine \
	-I../../lib/libevcs/include/frame \
	-I./src -I../ \
	-I../../lib/libucmq/src \
	-I../../lib/cJSON

LIB_DIR=-L/usr/local/lib -L../../lib
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++
LIBA=-ljson-c -lkv -lucmq -lcjson -lmini

#[=========>MARK<=========]#
MAIN_APP_SERV ?= gopath

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

HANDLE_MODEL = EVCORO
ifeq ($(MAIN_APP_SERV), gopath)
INC_DIR += -I/usr/include/mysql -I../../lib/libtopo -I ./
LIB_DIR += -L/usr/lib64/mysql
LIBS += -lmysqlclient
LIBA += -ltopo
CFLAGS += -DOPEN_TOPO
endif

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += sniff_evcoro_lua_api.o lj_sniff_util.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_sniff_cfg.o \
	   load_swift_cfg.o \
	   swift_cpp_api.o \
	   app_queue.o \
	   switch_queue.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CLEANFILES = mhooktrace.log


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) -c $< -o $@

$(BIN)/switch_queue.o : ../switch_queue.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/lj_sniff_util.o : ../lj_sniff_util.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/app_queue.o : ../app_queue.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_gopath
	rm -rf gopath
	rm -rf ./bin_gomile
	rm -rf gomile
	rm -rf ./bin_kkb
	rm -rf kkb
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data

mc:
	nohup valgrind --tool=memcheck --leak-check=full --verbose ./gopath &
