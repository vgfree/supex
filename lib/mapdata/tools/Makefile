BIN=./obj
SRC=./
INC_DIR=-I/usr/include/mysql -I../ -I../libmappmr -I../libmapsegment  -I../libmappoi -I../libmaptopo -I../utils -I./ -I../../json-c/ -I../../libmini/src
LIB_DIR=-L/usr/local/lib -L/usr/lib64/mysql -L../../
LIBS= -lmysqlclient -lm -ljson-c -lmini -ldl
CFLAGS ?= -Wall -g -std=c99 -D_GNU_SOURCE

OBJS = db_api.o	\
        map_utils.o     \
        pmr_cfg.o       \
        map_line_file.o \
        map_grid_list.o \
        map_grid_file.o \
        seg_cfg.o \
        map_seg.o \
        map_seg_file.o \
        map_poi_file.o \
        poi_cfg.o \
	topo_info.o \
	topo_cfg.o \
	topo_file.o \
	map_topo.o \


TAGS = gen_line_file	\
        gen_grid_file   \
        gen_sgid_file   \
        gen_poi_file   \
	gen_topo_file  \


all:$(TAGS)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

gen_line_file:gen_line_file.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gen_sgid_file:gen_sgid_file.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gen_grid_file:gen_grid_file.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gen_poi_file:gen_poi_file.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gen_topo_file:gen_topo_file.o $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS)
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

%.o:../libmappmr/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../libmapsegment/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../libmappoi/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../libmaptopo/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

%.o:../utils/%.c
	gcc  $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(OBJS)
	rm -rf $(TAGS)
	rm -f *.data

distclean:
	make clean

