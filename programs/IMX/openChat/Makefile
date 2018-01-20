INC_DIR=-I$(HOME_PATH)/lib/libcomm/ \
	-I$(HOME_PATH)/lib/iniparser/src \
	-I$(HOME_PATH)/lib/zeromq-4.2.1/include/ \
	-I$(HOME_PATH)/lib/libmini/include \
	-I$(HOME_PATH)/lib/libkv/release/ \
	-I./
LIB_DIR=-L/usr/local/lib -L$(HOME_PATH)/lib

INC_LIBS=-liniparser -lcomm -lzmq -lkv -lleveldb -lmini -lstdc++ -luuid -lpthread -ldl -lm

#[=========>MARK<=========]#
MAIN_APP_SERV ?= openChat

HANDLE_MODEL = EVCORO

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#
CFLAGS += -g -std=c99 -O1 -W -Wall



BIN=./bin_$(MAIN_APP_SERV)

SRV := $(MAIN_APP_SERV)
JOB += $(SRV)

OBJ = $(addprefix $(BIN)/, \
      main.o \
      \
      message_zmq_io.o \
      message_comm_io.o \
      message_gateway.o \
      message_sockfd_manage.o \
      login_zmq_io.o \
      login_comm_io.o \
      login_gateway.o \
      login_sockfd_manage.o \
      control_zmq_io.o \
      control_comm_io.o \
      control_gateway.o \
      control_sockfd_manage.o \
      \
      exchange.o \
      exc_cid_map.o \
      exc_gid_map.o \
      exc_message_dispatch.o \
      exc_status.o \
      exc_event_notify.o \
      exc_sockfd_manager.o \
      exc_uid_map.o \
      )



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) $(INC_LIBS) -pthread -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/main.o : main.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
$(BIN)/%.o : exchange/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
$(BIN)/%.o : gateway/message/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
$(BIN)/%.o : gateway/login/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
$(BIN)/%.o : gateway/control/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(MAIN_APP_SERV) $(BIN)

distclean:
	make clean
