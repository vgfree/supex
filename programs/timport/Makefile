BASE_SRC=./src
ADDITION_SRC=../../engine
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev \
	-I../../lib/hiredis -I../../lib/json-c -I../../lib/cJSON \
	-I../../lib/http-parser -I../../lib/libscco \
	-I../../lib/libucmq/src -I../../engine \
	-I../../engine/major -I../../engine/minor -I./src \
	-I../../lib/zookeeper-3.4.6/src/c/include \
	-I../../lib/zookeeper-3.4.6/src/c/generated

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/

LIBS=-lluajit-5.1 -lpthread -lrt -lm -luuid -lstdc++ 
LIBA=-ljson-c -lucmq -lcjson -lzookeeper_mt #-ljemalloc

#[=========>MARK<=========]#
MAIN_APP_SERV ?= timport

#CFLAGS += -DHAVE_TEST

ifeq ($(MAIN_APP_SERV), timport)
HANDLE_MODEL = EVUV
CFLAGS += -DMAX_STEP_COUNT=2
endif

ifeq ($(MAIN_APP_SERV), msgimport)
HANDLE_MODEL = EVUV
CFLAGS += -DMAX_STEP_COUNT=5
endif

ifeq ($(MAIN_APP_SERV), rtimport)
HANDLE_MODEL = EVUV
CFLAGS += -DMAX_STEP_COUNT=5
endif

ifeq ($(MAIN_APP_SERV), simimport)
HANDLE_MODEL = EVUV
CFLAGS += -DMAX_STEP_COUNT=5
endif


#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

#ADD_OBJ += sniff_evuv_cpp_api.o

#<--------------------------->#

BIN=bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
       timport_task.o \
       timport_api.o \
       timport_cfg.o \
       timport_evcb.o \
       timport_filter.o \
       tsdb.o \
       backup.o \
       misc.o \
       zk.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	@-if [ ! -d ./var/logs ];then mkdir -p var/logs var/backup; fi
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_timport
	rm -rf timport
	rm -rf ./bin_msgimport
	rm -rf msgimport
	rm -rf ./bin_rtimport
	rm -rf rtimport
	rm -rf ./bin_simimport
	rm -rf simimport

distclean:
	make clean
	rm -rf ./var/logs/*.log

