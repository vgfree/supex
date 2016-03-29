BASE_SRC=./src
#WITH_SRC=./cpp
INC_DIR=-I/usr/include/mysql -I/usr/local/include/luajit-2.0 -I../../../lib/libev -I../../../lib/json-c -I../../../lib/http-parser -I../../../lib/libscco -I../../../engine -I../../../engine/major -I../../../lib/libmini/include -I./src -I../../../lib/libevcoro/include -I../../../lib/cJSON -I./src/lib
LIB_DIR=-L/usr/local/lib -L/usr/lib64/mysql -L../../../engine/ -L./src/lib -L../../../lib
LIBS=-lmysqlclient -lluajit-5.1 -lpthread -lrt -lm
LIBA=-ljson-c -ltopo -lcjson

#[=========>MARK<=========]#
MAIN_APP_SERV ?= rrtopo

ifeq ($(MAIN_APP_SERV), rrtopo)
HANDLE_MODEL = LINE
endif

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

ADD_OBJ += smart_line_cpp_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_cfg.o \
	   binheap.o \
	   fetch_road.o \
	   calculate.o \
	   update_thread.o \
	   )

#WITH_OBJ = $(addprefix $(BIN)/, \
	   entry.o \
	   )

OBJ ?= $(BASE_OBJ) #$(WITH_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CLEANFILES = mhooktrace.log


all:libtopo $(JOB)
	#$(MAKE) -C ./src/lib
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

libtopo:
	cd ./src/lib && $(MAKE) LIBA=

$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


#$(BIN)/%.o : $(WITH_SRC)/%.c
#	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
#	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/devel

clean:
	$(MAKE) -C ./src/lib clean
	rm -rf ./bin_rrtopo
	rm -rf rrtopo
	rm -rf test/car.txt
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs

