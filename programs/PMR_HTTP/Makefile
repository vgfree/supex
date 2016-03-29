BASE_SRC=./src
INC_DIR=-I/usr/include/mysql -I/usr/local/include/luajit-2.0 \
	-I../../lib/libev -I../../lib/json-c -I../../lib/http-parser \
	-I../../lib/libscco -I../../engine -I../../engine/major -I./src  -I ./ \
	-I../../lib/mapdata/libmapsegment -I../../lib/mapdata/libmappmr

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/ -L/usr/lib64/mysql
LIBS=-lluajit-5.1 -lmysqlclient -lpthread -lrt -lm -lmapsegment -lmappmr 
LIBA=-ljson-c

#[=========>MARK<=========]#
MAIN_APP_SERV ?= pmrhttp

ifeq ($(MAIN_APP_SERV), pmrhttp)
HANDLE_MODEL = LINE
ADD_OBJ += match_road.o
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += smart_line_lua_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_cfg.o \
	   )

OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CLEANFILES = mhooktrace.log



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_pmrhttp
	rm -rf pmrhttp
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

