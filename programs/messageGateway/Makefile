INC_DIR=-I../../lib/libcomm/ -I../../lib/utils -I../../lib/zeromq-4.1.4/include/ -I../../lib/liblog
LIB_DIR=-L/usr/local/lib -L../../lib  -L../../lib/liblog/slog/lib -llog4cxx -lslog -lstdc++

INC_LIBS=../../lib/utils/lib/libutils.a ../../lib/libcomm/lib/libcomm.a ../../lib/libzmq.a

#[=========>MARK<=========]#
MAIN_APP_SERV ?= messageGateway

HANDLE_MODEL = LINE

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#



BIN=./bin_$(MAIN_APP_SERV)

SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CFLAGS += -g -std=c99

MAIN_OBJ = comm_io_wraper.c \
	   zmq_io_wraper.c \
	   upstream.c \
	   downstream.c \
	   zmq_pull_thread.c \
	   message_concentrator.c \
	   main.c

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) $(MAIN_OBJ) $(INC_LIBS) -pthread -o $@
#	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm $(MAIN_APP_SERV)
	rm -f $(CLEANFILES)
	rm -rf logs

distclean:
	make clean
	rm -rf ./logs/*.log
	
