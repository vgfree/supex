all:mfptppack.so prepare move

OBJ_DIR=./obj
SRC_DIR=./src

LDFLAGS =           $(ADDITION_CFLAGS)
LIBS =              $(ADDITION_LIBA)
LPATH =             $(ADDITION_LPATH)

OBJS = mfptp_pack.o 	\
	lua_mfptp.o 	\

mfptppack.so:$(OBJS)
	gcc $(LDFLAGS) -shared -fPIC $(LPATH) -o $@ $^ -I/usr/local/include/luajit-2.0 -L/usr/local/lib -lluajit-5.1 $(LIBS)

%.o: $(SRC_DIR)/%.c
	gcc -o $@ -c $^ -fPIC -I/usr/local/include/luajit-2.0 -L/usr/local/lib -lluajit-5.1 -Wall

move:
	mv mfptp_pack.o $(OBJ_DIR)
	mv lua_mfptp.o $(OBJ_DIR)
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(OBJS)
	rm -f mfptp_pack.so
prepare:
	-mkdir $(OBJ_DIR)

.PHONY:	all clean prepare move
