export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev \
	-I../../lib/json-c -I../../lib/http-parser -I../../lib/libscco \
	-I../../lib/libucmq/src -I../../engine -I../../engine/major \
	-I../../engine/minor -I./src -I../../lib/cJSON -I/usr/local/include \
	-I../../lib/hiredis/ -I../../lib/libkv/release -I/usr/include/mysql

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/ -L../../lib/hiredis/ -L/usr/lib64/mysql
LIBS=-lhiredis -lkv -lluajit-5.1 -lpthread -lrt -lm -lz -lstdc++  -lmysqlclient `pkg-config --cflags --libs protobuf`
LIBA=-ljson-c -lucmq -lcjson

#[=========>MARK<=========]#
MAIN_APP_SERV ?= gdgive

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS += -DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), gdgive)
HANDLE_MODEL = EVUV
CFLAGS += -D_GAODE
CFLAGS += -D_BJSH
#CFLAGS += -Wl,rpath=/usr/local/lib
else
HANDLE_MODEL = EVUV
#CFLAGS += -Wl,rpath=/usr/local/lib
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += sniff_evuv_cpp_api.o
ADD_OBJ += loadfile_tomem.o
ADD_OBJ += redis_kv.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   rr_cfg.o \
	   load_sniff_cfg.o \
	   load_swift_cfg.o \
	   swift_cpp_api.o \
	   add_session_cmd.o \
	   mirrtalk-transfer-data.pb.o \
	   cjson_topb.o \
	   mysql_ops.o \
	   filter.o \
	   count.o \
	   statistics_thread.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

#pblib:
#	$(MAKE) -C ./src staticlib

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	$(MAKE) -C $(BASE_SRC) M=$(PWD)
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(BASE_SRC)/%.cc
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	g++ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@ `pkg-config --cflags --libs protobuf`

push:
	git push origin HEAD:refs/for/devel

clean:
	rm -rf ./bin_gdgive
	rm -rf ./bin_bdgive
	rm -rf gdgive bdgive
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data
