SRC=./src
BIN=./bin
INC_DIR=-I../../lib/json-c -I../../lib/leveldb-1.19/include/ -I../../lib/zeromq-4.1.4/include/ -I./src
LIB_DIR=-L/usr/local/lib -L../../lib
LIBS=-lstdc++ -lrt -lpthread
LIBA=-ljson-c -lleveldb -lzmq -lsnappy

CFLAGS = -g -Wall -DDEBUG

LOGHIT=loghit

OBJ = $(addprefix $(BIN)/, \
	ldb.o \
	load_cfg.o \
	zmq_api.o \
	loghit.o \
)





$(LOGHIT):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@ 

$(BIN)/%.o : $(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

libs:
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./lib

clean:
	rm -rf $(LOGHIT)
	rm -rf $(BIN)

distclean:
	make clean
	$(MAKE) -C ./lib clean

push:
	git push origin HEAD:refs/for/master
