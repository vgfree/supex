SIN_SRC=./single
SEC_SRC=./subsection
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../../lib/libev \
	-I../../../lib/json-c -I../../../lib/http-parser -I../../../lib/libscco \
	-I../../../lib/libucmq/src -I../../../engine -I../../../engine/major \
	-I../../../lib/libkv/release/ \
	-I../../../engine/minor  \
	-I../../../lib/cJSON \
	-I../../../lib/libmini/include \
	-I../src -I./single -I./subsection -I./

LIB_DIR=-L/usr/local/lib -L../../../lib -L../../../engine/ 
LIBS=-lluajit-5.1 -lpthread -lrt -lm 
LIBA=-ljson-c -lucmq -lcjson -lkv
CPP_SERV ?= librrmodel.a
#[=========>MARK<=========]#

#CFLAGS +=-DSTORE_USE_UCMQ_AND_QUEUE

#ifeq ($(MAIN_APP_SERV), roadRank)
#HANDLE_MODEL = EVUV
#endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
#<------------------------>#

#ADD_OBJ += sniff_evuv_cpp_api.o

#<--------------------------->#

BIN=../bin_roadRank/bin_$(CPP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   custom_hash.o \
	   gps_info.o \
	   road_info.o \
	   rr_handle.o \
	   traffic_info.o \
	   traffic_model.o \
	   kv_imei.o \
	   kv_road_v2.o \
	   kv_cache.o \
       forward_imei.o \
       kv_speed.o \
	   )

SIN_OBJ = $(addprefix $(BIN)/, \
	   single_cfg.o \
	   single_model.o \
	   )

SEC_OBJ = $(addprefix $(BIN)/, \
	   subsection_cfg.o \
	   subsection_model_v2.o \
	   )

OBJ ?= $(BASE_OBJ) $(SIN_OBJ) $(SEC_OBJ)
SRV := $(CPP_SERV)
JOB = $(SRV)



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	ar -rcs $@ $^
	#gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o :./%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(SIN_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(SEC_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/devel

clean:
	rm -rf ../bin_roadRank/bin_model
	rm -rf $(CPP_SERV)

distclean:
	make clean
