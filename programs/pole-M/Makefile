SRV=pole-M

LOG=./logs
SRC=./src
BIN=./bin

INC_PATH=\
	 -I/usr/local/include/luajit-2.0 \
	 -I/usr/local/include \
	 -I../../lib/json-c \
	 -I../../lib/libev \
	 -I../../lib/libkv/release \
	 -I../../lib/libmemhook/src \
	 -I../../lib/libscco/include \
	 -I../../lib/libcoro/include \
	 -I../../lib/libevcoro/include \
	 -I../../lib/libevcs/include/engine \
	 -I../../lib/libevcs/include/frame \
	 -I./src -I../ \
	 -I../../lib/leveldb-1.18/include/ \
	 -I../../lib/zeromq-4.1.4/include/ \
	 -I../../lib/libmini/include/ \
	 -I../../lib/libxmq/include/ \
	 -I../../lib/libnetmod/include/

LIB_PATH=-L../../lib

LIBS=-lpthread -lrt -lm -ldl -lluajit-5.1 -lstdc++
LIBA=-lleveldb -ljson-c -lmini -levcoro -lxmq -lnetmod -lzmq -lsnappy -levcs

#<------------------------>#
HANDLE_MODEL = EVCORO
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#


OBJS=$(addprefix $(BIN)/, \
     pole_common.o \
     evt_hander.o \
     evt_worker.o \
     storage_impl.o \
     conf.o \
     load_swift_cfg.o \
     swift_cpp_api.o \
     main.o \
     )

JOB ?= $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJS)
	@-if [ ! -d $(LOG) ];then mkdir $(LOG); fi
	gcc $(CFLAGS) $^ $(LIB_PATH) $(LIBA) $(LIBS) -o $@

$(BIN)/%.o : $(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_PATH) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(SRV)

DEL:
	rm -rf $(LOG)/*.log nohup.out
	rm -rf ./data/* ./kvstate

distclean:
	make clean
	make DEL
