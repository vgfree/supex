BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c \
	-I../../lib/libkv/release \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco \
	-I../../lib/libcoro/include \
	-I../../lib/libevcoro \
	-I../../lib/libevcs \
	-I./src -I../ \
	-I../../lib/hiredis \
	-I../../lib/zookeeper-3.4.6/src/c/include \
	-I../../lib/zookeeper-3.4.6/src/c/generated \
	-I../../lib/leveldb-1.20/include \
	-I../../lib/libkv/src

LIB_DIR=-L/usr/local/lib -L../../lib

LIBS=-lluajit-5.1 -lstdc++ -lpthread -lrt -lm# -luuid
LIBA=-ljson-c -lkv -lcjson -lzookeeper_mt -lleveldb -lsnappy

#[=========>MARK<=========]#
MAIN_APP_SERV ?= tsdb

CFLAGS += -DOPEN_COMPRESSION

ifeq ($(MAIN_APP_SERV), tsdb)
HANDLE_MODEL = EVCORO
endif

ifeq ($(MAIN_APP_SERV), dfsdb)
HANDLE_MODEL = EVCORO
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#


#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
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

$(BIN)/ldb.o : ../ldb.c
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
