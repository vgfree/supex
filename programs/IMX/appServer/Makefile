BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I/usr/local/include/ \
	-I$(HOME_PATH)/lib/json-c \
	-I$(HOME_PATH)/lib/libmini/include \
	-I$(HOME_PATH)/lib/libkv/release \
	-I$(HOME_PATH)/lib/libmemhook/src \
	-I$(HOME_PATH)/lib/libscco \
	-I$(HOME_PATH)/lib/libcoro \
	-I$(HOME_PATH)/lib/libev \
	-I$(HOME_PATH)/lib/libevcoro \
	-I$(HOME_PATH)/lib/libevcs \
	-I$(HOME_PATH)/lib/iniparser/src \
	-I..//libappsrv \
	-I$(HOME_PATH)/lib/zeromq-4.2.1/include \
	-I./src -I../



LIB_DIR=-L/usr/local/lib -L$(HOME_PATH)/lib -L../
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++
LIBA=-ljson-c -lkv -lappsrv -liniparser -lzmq

#[=========>MARK<=========]#
MAIN_APP_SERV ?= ptop

ifeq ($(MAIN_APP_SERV), appServer)
HANDLE_MODEL = EVCORO
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

#ADD_OBJ += smart_evcoro_lua_api.o lj_smart_util.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
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
	rm -rf ./bin_appServer
	rm -rf appServer
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

