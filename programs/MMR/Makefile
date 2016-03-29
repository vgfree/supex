BASE_SRC=./src
CAPP_SRC=./app
CAPI_SRC=./api
ADDITION_SRC=../../engine
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser \
	-I../../lib/libscco -I../../engine -I../../engine/major -I../../engine/minor \
	-I../../lib/mapdata/libmappoi \
	-I../../lib/libkv/release/ \
	-I./src -I./app -I./api \
	-I../../lib/libucmq/src \
	-I../../lib/libmemhook/src/


LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lstdc++ -lpthread -lrt -lm 
LIBA=-ljson-c -lucmq

#[=========>MARK<=========]#
MAIN_APP_SERV ?= drisamp

#<--drisamp/drimode/rtmiles/rta需要连接lru库-->#
#ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),drisamp drimode rtmiles rta))
#endif

#CFLAGS +=-DSTORE_USE_UCMQ
#CFLAGS +=-DSTORE_USE_QUEUE
CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

ifeq ($(MAIN_APP_SERV), drisamp)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), drimode)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), rtmiles)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), rta)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), goby)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), drisampapi)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), drimodeapi)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), rtmilesapi)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), rtaapi)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_APP_SERV), gobyapi)
HANDLE_MODEL = LINE
endif

#<------goby的宏--->#
ifeq ($(MAIN_APP_SERV), goby)
#<--goby需要使用mysql和topoi-->#
INC_DIR += -I/usr/include/mysql -I../../lib/mapdata/libmappoi -I ./
LIB_DIR += -L /usr/lib64/mysql
LIBS += -lmysqlclient -lmappoi
CFLAGS += -DGOBY
endif
ifeq ($(MAIN_APP_SERV), gobyapi)
#<--goby需要使用mysql和topoi-->#
INC_DIR += -I/usr/include/mysql -I../../lib/mapdata/libmappoi -I ./
LIB_DIR += -L /usr/lib64/mysql
LIBS += -lmysqlclient -lmappoi
CFLAGS += -DGOBY
endif
#<------------------------>#

-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk


BIN=./bin_$(MAIN_APP_SERV)

#<--drisamp/drimode/rtmiles需要使用的.o文件 -->#
ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),drisamp drimode rtmiles rta goby))
ifeq ($(HANDLE_MODEL), SCCO)
ADD_OBJ += smart_scco_cpp_api.o
ADD_OBJ += sniff_scco_lua_api.o
endif
ifeq ($(HANDLE_MODEL), LINE)
ADD_OBJ += smart_line_cpp_api.o
ADD_OBJ += sniff_line_lua_api.o
endif
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   app_main.o \
	   entry.o \
	   add_session_cmd.o \
	   load_smart_cfg.o \
	   load_swift_cfg.o \
	   load_sniff_cfg.o \
	   swift_cpp_api.o \
	   )
endif

#<--drisampapi/drimodeapi/rtmilesapi需要使用的.o文件 -->#
ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),drisampapi drimodeapi rtmilesapi rtaapi gobyapi))
ifeq ($(HANDLE_MODEL), SCCO)
ADD_OBJ += smart_scco_lua_api.o
endif
ifeq ($(HANDLE_MODEL), LINE)
ADD_OBJ += smart_line_lua_api.o
endif
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   api_main.o \
	   entry.o \
	   load_smart_cfg.o \
	   )
endif

#<-- goby使用的.o文件 -->#
ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),goby gobyapi))
BASE_OBJ += $(addprefix $(BIN)/, \
	  lua_mappoi.o \
	  )
endif


OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)
CLEANFILES = mhooktrace.log



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@ 
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@sh init/init.sh $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(CAPP_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(CAPI_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


load:
ifeq ($(MMR), default)
	@echo -e $(RED)"ERROR operate!!!\n"$(GREEN)"please use like:"
	@echo -e $(YELLOW)$(foreach obj,$(SRV),"\t-->>make load MMR="$(obj)"\n")$(NONE)
else
	./$(MMR) -c $(MMR)_conf.json &
	sleep 30
	luajit init/$(MMR)_load.lua
	sleep 2
	killall $(MMR)
endif
push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_drisamp
	rm -rf ./bin_drimode
	rm -rf ./bin_goby
	rm -rf ./bin_rtmiles
	rm -rf ./bin_rta
	rm -rf drisamp drimode goby rtmiles rta
	rm -rf ./bin_drisampapi
	rm -rf ./bin_drimodeapi
	rm -rf ./bin_gobyapi
	rm -rf ./bin_rtmilesapi
	rm -rf ./bin_rtaapi
	rm -rf drisampapi drimodeapi gobyapi rtmilesapi rtaapi
	rm -f $(CLEANFILES)
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./lua/drisamp/data/
	rm -rf ./lua/drimode/data/
	rm -rf ./lua/goby/data/
	rm -rf ./lua/rtmiles/data/
	rm -rf ./lua/rta/data/

