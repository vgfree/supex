
INC_DIR=\
    -I/usr/local/include \
    -I../../lib/json-c/ \
    -I../../lib/zeromq-4.1.4/include/ \
    -I../../lib/libmini/include/ \
    -I../../lib/libnetmod/include/ \
    -I../../engine/ \
    -I./src

LIB_DIR=\
    -L../../lib \
    -L/usr/local/lib/ \
    -L/usr/lib64/mysql
    
LIBS=-lmysqlclient -lpthread -lrt -lm -lstdc++
LIBA=-ljson-c -lmini -lnetmod -lzmq

# [args] Running mode, DEBUG: debug mode. RELEASE: release mode.
ifeq ($(COMPILE), RELEASE)
  CFLAGS=-O2 -DNDEBUG -Wall
else
  CFLAGS=-g -O0
endif

#/////////////////////DES_EXEC////////////////////////

SRV=pole-S
MAIN_APP_SERV ?= $(SRV)

BIN=./bin
LOG=./logs
DEPS=../../lib
BASE_SRC=./src

BASE_OBJ=$(addprefix $(BIN)/,\
    clog.o \
    main.o \
    register.o \
    parser.o \
    )

DEPS_OBJ=$(addprefix $(DEPS)/,\
    libjson-c.a \
    libzmq.a \
    )


OBJ ?= $(BASE_OBJ)
JOB += $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d $(LOG) ];then mkdir $(LOG); fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin $(SRV)

distclean:
	make clean
	rm -rf $(LOG) 
