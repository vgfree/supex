BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I/usr/local/include/ \
	-I../../lib/libev \
	-I../../lib/json-c \
	-I../../lib/libmini/include \
	-I../../lib/libkv/release \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco \
	-I../../lib/libcoro \
	-I../../lib/libevcoro \
	-I../../lib/libevcs \
	-I./src -I../



LIB_DIR=-L/usr/local/lib -L../../lib
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++
LIBA=-ljson-c

#[=========>MARK<=========]#
MAIN_APP_SERV ?= ptop

ifeq ($(MAIN_APP_SERV), driviewapi)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), ptop)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), weibo-S)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), spxapi)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), dfsapi)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), rmsapi)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), hlsapi)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), robais)
HANDLE_MODEL = EVCORO
endif
ifeq ($(MAIN_APP_SERV), tsearchapi)
HANDLE_MODEL = EVCORO
INC_DIR += -I../../lib/zookeeper-3.4.6/src/c/include \
	   -I../../lib/zookeeper-3.4.6/src/c/generated
CFLAGS += -DOPEN_ZOOKEEPER
ADD_OBJ += zk.o lua_zk.o
LIBA += -lzookeeper_mt
endif
ifeq ($(MAIN_APP_SERV), msgsearchapi)
HANDLE_MODEL = EVCORO
INC_DIR += -I../../lib/zookeeper-3.4.6/src/c/include \
	   -I../../lib/zookeeper-3.4.6/src/c/generated
CFLAGS += -DOPEN_ZOOKEEPER
ADD_OBJ += zk.o lua_zk.o
LIBA += -lzookeeper_mt
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += smart_evcoro_lua_api.o lj_smart_util.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_cfg.o \
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
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/lj_smart_util.o : ../lj_smart_util.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_driviewapi
	rm -rf ./bin_ptop
	rm -rf ./bin_weibo-S
	rm -rf ./bin_spxapi
	rm -rf ./bin_dfsapi
	rm -rf ./bin_rmsapi
	rm -rf ./bin_hlsapi
	rm -rf ./bin_robais
	rm -rf ./bin_tsearchapi
	rm -rf ./bin_msgsearchapi
	rm -rf driviewapi ptop weibo-S spxapi dfsapi rmsapi hlsapi robais tsearchapi msgsearchapi
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

