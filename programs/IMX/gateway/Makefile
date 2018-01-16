INC_DIR=-I$(HOME_PATH)/lib/libcomm/ \
	-I$(HOME_PATH)/lib/iniparser/src \
	-I$(HOME_PATH)/lib/zeromq-4.2.1/include/ \
	-I$(HOME_PATH)/lib/libmini/include
LIB_DIR=-L/usr/local/lib -L$(HOME_PATH)/lib -L../

INC_LIBS=-liniparser -lcomm -lzmq -lmini -lstdc++ -lm

#[=========>MARK<=========]#
MAIN_APP_SERV ?= gateway

HANDLE_MODEL = EVCORO

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#



BIN=./bin_$(MAIN_APP_SERV)

SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CFLAGS += -g -std=c99

MAIN_OBJ = main.c \
	   message/message_zmq_io.c \
	   message/message_comm_io.c \
	   message/message_gateway.c \
	   message/message_sockfd_manage.c \
	   login/login_zmq_io.c \
	   login/login_comm_io.c \
	   login/login_gateway.c \
	   login/login_sockfd_manage.c \
	   control/control_zmq_io.c \
	   control/control_comm_io.c \
	   control/control_gateway.c \
	   control/control_sockfd_manage.c

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
	rm -rf $(MAIN_APP_SERV)
	#rm -rf $(CLEANFILES)
	rm -rf logs

distclean:
	make clean
	rm -rf ./logs/*.log
	
