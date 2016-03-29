BIN=./obj
SRC=./
INC_DIR=-I/usr/include/mysql -I../ -I../../../json-c/ -I../../utils -I../../../libmini/src -I../../../libmini/src/slog/
LIB_DIR=-L/usr/local/lib -L../ -L/usr/lib64/mysql -L../../..
LIBS= -ljson-c -lmysqlclient  -lm -lmini -ldl
CFLAGS ?= -Wall -g -std=c99 -D_GNU_SOURCE

OBJS = db_api.o	\
	poi_cfg.o	\
	map_poi_file.o	\
	map_poi.o	\
	poi_test.o      \

TAGS = poi_test	\

all:$(TAGS)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

poi_test:poi_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

%.o:../%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../../utils/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


clean:
	rm -f $(OBJS)
	rm -rf $(TAGS)

distclean:
	make clean

