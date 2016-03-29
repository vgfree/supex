BIN=./obj
SRC=./
INC_DIR=-I/usr/include/mysql -I../ -I../../../json-c/ -I../../utils -I../../../libmini/src
LIB_DIR=-L/usr/local/lib -L../ -L/usr/lib64/mysql -L../../..
LIBS= -ljson-c -lmysqlclient -lm -lmini -ldl
CFLAGS ?= -Wall -g -std=c99 -D_GNU_SOURCE

OBJS = db_api.o	\
	pmr_cfg.o	\
        map_line_file.o \
	map_line.o	\
        map_grid_list.o \
        map_grid_file.o \
	map_grid.o	\
        map_utils.o\
        map_pmr.o\

TAGS = db_test	\
	grid_test	\
	cfg_test	\
	pmr_test	\
	line_test	\

all:$(TAGS)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

db_test:db_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

cfg_test:cfg_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

grid_test:grid_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

pmr_test:pmr_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


line_test:line_test.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

%.o:../%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../../utils/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


clean:
	rm -f *.o
	rm -f $(OBJS)
	rm -rf $(TAGS)

distclean:
	make clean

