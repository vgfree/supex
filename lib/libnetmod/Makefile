SRV=libnetmod.a

BIN=./bin
INC=./include
BASE_SRC=./src
BASE_INC=-I$(INC) -I../libmini/include \
	-I../zeromq-4.1.4/include/ \
	-I../libevcs/include/engine \
	-I/usr/local/include/luajit-2.0 \
	-I/usr/local/include/

LIB_INC=-L. -L../
LIBS=-lpthread -lrt -lstdc++
LIBA=-lmini -lzmq -ldl

BASE_OBJ=$(addprefix $(BIN)/,\
    netmod.o \
    )

OBJ ?= $(BASE_OBJ)

JOB ?= $(SRV) 

all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

###======= LIB  =========###

$(SRV):$(OBJ)
	@echo -e "###======= make library =========###"
	@echo -e "Compiler to static library."
	ar crv $@ $^

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(BASE_INC) -c $< -o $@

###======= CLEAN =========###

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(BIN_TEST) $(SRV) $(TEST)

distclean:
	make clean

