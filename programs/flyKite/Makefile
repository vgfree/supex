BASE_SRC=./src
ADDITION_SRC=../../engine
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c \
	-I../../lib/http-parser -I../../lib/libscco \
	-I../../lib/libucmq/src -I../../engine -I../../engine/major \
	-I../../engine/minor -I./src \
	-I../../lib/libmemhook/src/

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/

LIBS=-lluajit-5.1 -lpthread -lrt -lm 
LIBA=-ljson-c -lucmq

#[=========>MARK<=========]#
MAIN_APP_SERV ?= gopath

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), gopath)
HANDLE_MODEL = LINE
INC_DIR += -I/usr/include/mysql -I../../lib/libtopo -I ./
LIB_DIR += -L/usr/lib64/mysql
LIBS += -lmysqlclient
LIBA += -ltopo
CFLAGS += -DOPEN_TOPO
endif

ifeq ($(MAIN_APP_SERV), gomile)
HANDLE_MODEL = LINE
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += sniff_line_lua_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_sniff_cfg.o \
	   load_swift_cfg.o \
	   swift_cpp_api.o \
	   add_session_cmd.o \
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

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_gopath
	rm -rf gopath
	rm -rf ./bin_gomile
	rm -rf gomile
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data

mc:
	nohup valgrind --tool=memcheck --leak-check=full --verbose ./gopath &
