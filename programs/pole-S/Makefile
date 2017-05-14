SRV=pole-S

LOG=./logs
SRC=./src
BIN=./bin

INC_PATH=\
	 -I/usr/local/include/luajit-2.0 \
	 -I/usr/local/include \
	 -I../../lib/json-c \
	 -I../../lib/zeromq-4.2.1/include/ \
	 -I../../lib/libmini/include/ \
	 -I../../lib/libnetmod/include/ \
	 -I../../lib/libcoro/include \
	 -I../../lib/libevcoro/include \
	 -I../../lib/libevcs/include \
	 -I../../lib/libevcs/include/engine \
	 -I../../lib/libevcs/include/frame \
	 -I./src -I../

LIB_PATH=-L../../lib \
    -L/usr/local/lib/ \
    -L/usr/lib64/mysql

LIBS=-lmysqlclient -lpthread -lrt -lm -lstdc++ -ldl
LIBA=-ljson-c -lmini -lnetmod -lzmq -levcs



OBJS=$(addprefix $(BIN)/, \
    main.o \
    register.o \
    conf.o \
    )


JOB ?= $(SRV)

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

$(SRV):$(OBJS)
	@-if [ ! -d $(LOG) ];then mkdir $(LOG); fi
	gcc $(CFLAGS) $^ $(LIB_PATH) $(LIBA) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/%.o : $(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_PATH) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(SRV)

distclean:
	make clean
	rm -rf $(LOG) 
