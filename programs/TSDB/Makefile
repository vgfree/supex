BASE_SRC=./src
ADDITION_SRC=../../engine
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev \
	-I../../lib/hiredis -I../../lib/json-c -I../../lib/cJSON \
	-I../../lib/http-parser -I../../lib/libscco \
	-I../../lib/libucmq/src -I../../engine \
	-I../../engine/major -I../../engine/minor -I./src \
	-I../../lib/zookeeper-3.4.6/src/c/include \
	-I../../lib/zookeeper-3.4.6/src/c/generated \
	-I../../lib/leveldb-1.18/include \
	-I../../lib/libkv/src

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/

LIBS=-lluajit-5.1 -lpthread -lrt -lm -luuid -lstdc++ 
LIBA=-ljson-c -lucmq -lcjson -lzookeeper_mt -lleveldb -lsnappy

#[=========>MARK<=========]#
MAIN_APP_SERV ?= tsdb

CFLAGS += -DOPEN_COMPRESSION

CFLAGS += -DOPEN_KV_EXPIRE

ifeq ($(MAIN_APP_SERV), tsdb)
HANDLE_MODEL = EVUV
endif

ifeq ($(MAIN_APP_SERV), dfsdb)
HANDLE_MODEL = EVUV
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#


#<--------------------------->#

BIN=bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   tsdb_cfg.o \
	   tsdb_api.o \
	   tsdb_ldb.o \
	   tsdb_kv.o \
	   tsdb_entry.o \
	   load_smart_cfg.o \
	   load_swift_cfg.o \
	   smart_cpp_api.o \
	   swift_cpp_api.o \
	   ldb.o \
	   ldb_priv.o \
	   binlog.o \
	   slave.o \
	   zk.o \
	   misc.o \
	   kv_priv.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./var/logs ];then mkdir -p var/logs var/data; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/ldb.o : ../../engine/ldb.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_tsdb
	rm -rf tsdb
	rm -rf ./bin_dfsdb
	rm -rf dfsdb

distclean:
	make clean
	rm -rf ./var/logs/*.log
