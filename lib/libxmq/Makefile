SRV=libxmq.a
TEST=mixmq

BIN=./bin
INC=./include
BASE_SRC=./src
BASE_INC=-I$(INC) -I../libmini/include \
	-I../libmini/include \
	-I../libevcs \
	-I/usr/local/include/luajit-2.0 \
	-I/usr/local/include/

BIN_TEST=./bin_test
TEST_SRC=./test
TEST_INC=$(BASE_INC) \
	-I../leveldb-1.19/include/ \
	-I../zeromq-4.1.4/include

LIB_INC=-L. -L../
LIBS=-lpthread -lrt -lstdc++
LIBA=-lmini -lxmq -lleveldb -lsnappy -lzmq -levcs -ldl

BASE_OBJ=$(addprefix $(BIN)/,\
    xmq_csv.o \
    xmq_msg.o \
    xmq.o \
    )

TEST_OBJ=$(addprefix $(BIN_TEST)/, \
    ldb_cb.o  \
    main.o \
    )

OBJ ?= $(BASE_OBJ)
OBJ_TEST ?= $(TEST_OBJ)

JOB ?= $(SRV) #$(TEST)


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

###======= Test =========###

$(TEST):$(OBJ_TEST)
	gcc $(CFLAGS) $^ $(LIB_INC) -o $@ $(LIBA) $(LIBS)

$(BIN_TEST)/%.o : $(TEST_SRC)/%.c
	@-if [ ! -d $(BIN_TEST) ]; then mkdir $(BIN_TEST); fi
	gcc $(CFLAGS) $(TEST_INC) -c $< -o $@

###======= LIB  =========###

$(SRV):$(OBJ)
	@echo -e "###======= make library =========###"
	@echo -e "Compiler to static library."
	#@echo -e "<<":`ls $(INC)`">>"
	#@-if [ -z "`ls $(INC)`" ];then cp $(BASE_SRC)/*.h $(INC); fi
	ar crv $@ $^
	@echo -e "###======= make test =========###"

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	@-if [ ! -d $(INC) ];then mkdir $(INC); fi
	gcc $(CFLAGS) $(BASE_INC) -c $< -o $@

###======= CLEAN =========###

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(BIN_TEST) $(SRV) $(TEST)

distclean:
	make clean
	rm -rf ./ldb ./kvstate ./kvdata.log

