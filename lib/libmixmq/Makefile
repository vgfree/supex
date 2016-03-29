
DEPS=..
LIB_INC=-L$(DEPS)

SRV=$(DEPS)/libmixmq.a
TEST=mixmq_test

BIN=./bin
BASE_SRC=./src
BASE_INC=-I./include -I../libmini/include 

BIN_TEST=./bin_test
TEST_SRC=./app_demo
TEST_INC=-I./include -I../libmini/include -I../leveldb-1.18/include/ 

LIBS=-lpthread 
LIBA=-lmini -lmixmq -lleveldb -lsnappy -ldl

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

JOB ?= $(SRV) $(TEST)


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

###======= Test =========###

$(TEST):$(OBJ_TEST)
	g++ $(CFLAGS) $^ $(LIB_INC) -o $@ $(LIBA) $(LIBS)

$(BIN_TEST)/%.o : $(TEST_SRC)/%.c
	@-if [ ! -d $(BIN_TEST) ]; then mkdir $(BIN_TEST); fi
	g++ $(CFLAGS) $(TEST_INC) -c $< -o $@

###======= LIB  =========###

$(SRV):$(OBJ)
	@echo -e "###======= make library =========###"
	@echo -e "Compiler to static library."
	ar crv $@ $^
	@echo -e "###======= make test =========###"

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	g++ $(CFLAGS) $(BASE_INC) -c $< -o $@

###======= CLEAN =========###

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf $(BIN) $(BIN_TEST) $(SRV) $(TEST)

distclean:
	make clean
	rm -rf ./data

