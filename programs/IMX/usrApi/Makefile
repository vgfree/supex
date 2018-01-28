INC_DIR=-I$(HOME_PATH)/lib/libcomm/ \
	-I$(HOME_PATH)/lib/iniparser/src \
	-I$(HOME_PATH)/lib/libmini/include \
	-I$(HOME_PATH)/lib/libkv/release/ \
	-I/usr/local/include/luajit-2.0 \
	-I./
LIB_DIR=-L/usr/local/lib -L$(HOME_PATH)/lib

INC_LIBS=-liniparser -lcomm -lkv -lleveldb -lmini -lluajit-5.1 -lstdc++ -lpthread -ldl -lm
	

#[=========>MARK<=========]#
MAIN_APP_SERV ?= usrApi

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
      usrapi_i_wrap.o \
      usrapi_o_wrap.o \
      )



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) $(INC_LIBS) -pthread -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : src/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(MAIN_APP_SERV) $(BIN)

distclean:
	make clean
