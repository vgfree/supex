BASE_SRC=./src
WITH_SRC=./cpp
HAVE_SRC=./cpp/mfptp
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev \
	-I../../lib/hiredis -I../../lib/libtopo -I/usr/include/mysql \
	-I../../lib/json-c -I../../lib/http-parser -I../../lib/libscco \
	-I../../lib/libucmq/src -I../../engine -I../../engine/major \
	-I../../engine/minor -I./src -I./cpp -I./cpp/mfptp -I../../lib/sqlite-autoconf-3090200/ -I../../lib/zeromq-4.2.1/include/

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lpthread -lrt -lm -ldl -lsqlite3 -lz -lssl -lstdc++
LIBA=-ljson-c -lucmq -lzmq -lhiredis -ltopo

#[=========>MARK<=========]#
MAIN_APP_SERV ?= mfptpServer

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), mfptpServer)
HANDLE_MODEL = EVUV
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#


BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   route_cfg.o \
	   load_mfptp_cfg.o \
	   mfptp_cpp_api.o \
	   sqlite_api.o \
	   user_rbtree.o \
	   channel_rbtree.o \
	   user_topo.o \
           zmq_weibo_forward.o \
	   main.o \
	   \
	   mfptp_api.o \
	   mfptp_evcb.o \
	   mfptp_task.o \
           mfptp_callback.o \
	   mfptp_parser.o \
           zmqskt.o \
	   mfptp_users_rbtree.o \
	   x_utils.o \
           redis_status.o \
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

$(BIN)/%.o : $(WITH_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(HAVE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_mfptpServer
	rm -rf mfptpServer
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./mq_data
