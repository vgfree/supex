BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c \
	-I../../lib/libkv/release \
	-I../../lib/hiredis \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco \
	-I../../lib/libcoro/include \
	-I../../lib/libevcoro \
	-I../../lib/libevcs/ \
	-I../../lib/libevcs/engine \
	-I../../lib/libevcs/frame \
	-I../../lib/leveldb/include \
	-I../../lib/libcomm \
	-I./src -I../



LIB_DIR=-L/usr/local/lib -L../../lib -L../../lib/hiredis/
LIBS=-lluajit-5.1 -lstdc++ -lpthread -lrt -lm -lhiredis
LIBA=-lucmq -lcjson -lmini -ljson-c -lkv -lleveldb

#[=========>MARK<=========]#
MAIN_APP_SERV ?= mfptpSvp


ifeq ($(MAIN_APP_SERV), mfptpSvp)
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
	   load_alive_cfg.o \
	   alive_cpp_api.o \
	   load_sniff_cfg.o \
	   sniff_evcoro_lua_api.o \
	   lj_sniff_util.o \
	   keyval.o \
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
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/lj_sniff_util.o : ../lj_sniff_util.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_mfptpSvp
	rm -rf mfptpSvp
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data
