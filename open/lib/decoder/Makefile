all: prepare libluadecoder.so
	cp libluadecoder.so ../

OBJ_DIR=./obj
SRC_DIR=./src

LDFLAGS = 	    $(ADDITION_CFLAGS)
LIBS =		    $(ADDITION_LIBA)
LPATH = 	    $(ADDITION_LPATH)

libluadecoder.so: $(OBJ_DIR)/decode_impl.o $(OBJ_DIR)/lua_decoder.o 
	gcc $(LDFLAGS) -shared -fPIC $(LPATH) -o $@ $^ -lluajit-5.1 $(LIBS)

$(OBJ_DIR)/lua_decoder.o: $(SRC_DIR)/lua_decoder.c
	gcc -fPIC -I/usr/local/include/luajit-2.0/ -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/decode_impl.o: $(SRC_DIR)/decode_impl.c
	gcc -fPIC -c $< -o $@

mc:
	-rm -f nohup.out
	nohup valgrind --tool=memcheck --leak-check=full --verbose luajit ./test.lua

clean:
	rm -rf $(OBJ_DIR) *.so

prepare:
	-mkdir $(OBJ_DIR) 

.PHONY: all clean prepare
