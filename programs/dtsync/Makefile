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
	-I../../lib/cJSON \
	-I../../lib/zeromq-4.1.4/include/

LIB_DIR=-L/usr/local/lib -L../../lib
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++
LIBA=-lcjson -lkv -lmini -lucmq -lzmq -ljson-c 

#[=========>MARK<=========]#
MAIN_APP_SERV ?= dtsync

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS += -DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), dtsync)
HANDLE_MODEL = EVCORO
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += sniff_evcoro_cpp_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   rr_cfg.o \
	   load_sniff_cfg.o \
	   load_swift_cfg.o \
	   swift_cpp_api.o \
	   cqueue.o \
	   send_data.o \
	   app_queue.o \
	   redis_parse.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) -c $< -o $@

$(BIN)/app_queue.o : ../app_queue.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_dtsync
	rm -rf dtsync
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data

TAR:
	tar -zcvf dtsync_x86_64_`date +"%y%m%d"`_tar.gz dtsync dtsync_conf.json src mq_data test Makefile logs --exclude *.log

