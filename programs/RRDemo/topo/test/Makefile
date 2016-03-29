BASE_SRC=..
INC_DIR=-I../
LIB_DIR=
LIBS=
CFLAGS= -g 

MAIN_APP_SERV ?= test

BIN=./bin_$(MAIN_APP_SERV)

binheap_OBJ = $(addprefix $(BIN)/, \
	      binheap.o \
	      test_binheap.o \
	      )

test_binheap:$(binheap_OBJ)
	gcc $^ -o test_binheap

$(BIN)/%.o : $(BASE_SRC)/binheap.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
#$(BIN)/%.o : ./test_binheap.c
$(BIN)/test_binheap.o : ./test_binheap.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $^ -o $@

clean:
	rm -rf $(BIN)
	rm -rf test_binheap
