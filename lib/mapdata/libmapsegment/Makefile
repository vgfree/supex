BIN=./obj
SRC=./
INC_DIR=-I../../json-c -I/usr/include/mysql -I../utils/ -I../../libmini/src
LIB_DIR=-L/usr/local/lib -L../ -L/usr/lib64/mysql
LIBS=-lrt -lm -lz -lmysqlclient  -lmini -ldl
LIBA=#-ljson-c

TAGS = libmapsegment.a

LIBA += $(EXPORT_LIBA)
CFLAGS += $(EXPORT_CFLAGS)
CFLAGS ?= -Wall -g

OBJ = $(addprefix $(BIN)/, \
	seg_cfg.o \
	db_api.o \
      	map_utils.o \
	map_seg_file.o \
	map_seg.o \
      )


all:$(TAGS)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

libmapsegment.a:$(OBJ)
	ar -rcs $@ $^
	#gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@

$(BIN)/%.o:$(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o:../utils/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


clean:
	rm -rf $(BIN)
	rm -rf $(TAGS)

distclean:
	make clean
