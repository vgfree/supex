BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c \
	-I../../lib/libmini/include \
	-I../../lib/libkv/release \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco/include \
	-I../../lib/libcoro/include \
	-I../../lib/libevcoro/include \
	-I../../lib/libevcs/include/engine \
	-I../../lib/libevcs/include/frame \
	-I./src -I../ \
	-I../../lib/cJSON \
	-I/usr/include/mysql \
        -I../../lib/libucmq/src


LIB_DIR=-L/usr/local/lib -L../../lib -L /usr/lib64/mysql
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++ -lmysqlclient
LIBA=-ljson-c -lkv -levcs -lmini -ldl -lucmq -lcjson

#[=========>MARK<=========]#
MAIN_APP_SERV ?= releaseServer

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
#CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), releaseServer)
HANDLE_MODEL = EVCORO
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)

CFLAGS +=-D_GNU_SOURCE
#<------------------------>#

ifeq ($(DATA_LOAD_TYPE), cfg)
ADD_OBJ += cfg_imei.o	
endif

ifeq ($(DATA_LOAD_TYPE), mysql)
ADD_OBJ += mysql_imei.o
endif

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
           async_comm.o  cq_list.o    grp_pthread.o  jtt_business.o     jtt_comm.o         jtt_package.o  log.o         pkg_crc.o     pkg_sec.o  worker_mem.o \
           data_json.o   jtt_body.o   jtt_client.o   jtt_json_to_gps.o  load_swift_cfg.o   out_pthread.o  pkg_escape.o  worker.o \
           )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CLEANFILES = mhooktrace.log


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./log ];then mkdir log; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_releaseServer
	rm -rf releaseServer
	rm -f $(CLEANFILES)
	@-if [ ! -d ./log ];then mkdir log; fi

distclean:
	make clean
	rm -rf ./log/*.log
	rm -rf ./mq_data

mc:
	nohup valgrind --tool=memcheck --leak-check=full --verbose ./run.sh &
