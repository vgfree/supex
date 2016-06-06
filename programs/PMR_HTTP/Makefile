BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I../../lib/libev \
	-I../../lib/json-c \
	-I../../lib/libkv/release \
	-I../../lib/libmemhook/src \
	-I../../lib/libscco/include \
	-I../../lib/libcoro/include \
	-I../../lib/libevcoro/include \
	-I../../lib/libevcs/include/engine \
	-I../../lib/libevcs/include/frame \
	-I./src -I../ \
	-I/usr/include/mysql \
	-I../../lib/libtopo \
	-I../../lib/mapdata/libmappmr -I../../lib/mapdata/utils -I../../lib/libmini/include	\
	-I../../lib/mapdata/libmapsegment -I../../lib/mapdata/libmaptopo	\
	-I./cpp

LIB_DIR=-L/usr/local/lib -L../../lib -L/usr/lib64/mysql
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lmysqlclient
LIBA=-ljson-c -lmappmr -lmapsegment -lmaptopo -lkv

#[=========>MARK<=========]#
MAIN_APP_SERV ?= pmrhttp

ifeq ($(MAIN_APP_SERV), pmrhttp)
HANDLE_MODEL = EVCORO
ADD_OBJ += match_road.o
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
	rm -rf ./bin_pmrhttp
	rm -rf pmrhttp
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

