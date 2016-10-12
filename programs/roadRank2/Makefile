BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I../../lib/libev \
	-I../../lib/json-c \
	-I../../programs \
	-I../../lib/http-parser \
	-I../../lib/libscco/include\
	-I../../lib/libucmq/src \
	-I../../lib/libevcoro/include \
	-I../../lib/libevcs/include/ \
	-I../../lib/libkv/release/ \
	-I../../lib/libmini/include \
	-I./src \
	-I../../lib/cJSON \
	-I./cpp \
	-I./cpp/single \
	-I./cpp/subsection

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/ -L./cpp
LIBS=-lluajit-5.1 -lpthread -lrt -lm 
LIBA=-ljson-c -lucmq  -lkv -lrrmodel -lcjson

#[=========>MARK<=========]#
MAIN_APP_SERV ?= roadRank

CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
#CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), roadRank)
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
	   redis_parse.o \
	   app_queue.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)



all:model $(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

model:
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	cd ./cpp && $(MAKE)

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/app_queue.o : ../app_queue.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

run:
	-killall -9 roadRank
	nohup ./roadRank &
clean:
	$(MAKE) -C ./cpp clean
	rm -rf ./bin_roadRank
	rm -rf roadRank
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data
