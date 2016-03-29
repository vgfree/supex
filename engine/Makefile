#<------------------------>#
include ../share.mk
#<------------------------>#

INC_DIR=-I/usr/local/include/luajit-2.0 -I../lib/libev -I../lib/http-parser \
	-I../lib/libscco -I./major -I./minor -I. \
	-I../lib/libmemhook/src -I../lib/libkv/src \
	-I../lib/libevcoro/include \
	-I../lib/libmini/src \
	-I../lib/zeromq-4.1.4/include

LIB_DIR=-L/usr/local/lib -L../lib
LIBS=-lluajit-5.1 -lpthread -lcrypto -lrt -lm

#---------------------------------------
ECHO = @echo -e
COMPILER = gcc
AC = @ar -rcs

COLOR_OBJ =	$(CYAN)
COLOR_TAG =	$(YELLOW)
COLOR_TXT = $(GREEN)
COLOR_NONE = $(NONE)
COLOR_RED =$(RED)

define compile_obj
	$(ECHO) $(COLOR_TXT)"\t\t- COMPILE\t===>\t"$(COLOR_OBJ)"$(1:.c=.o)"$(COLOR_NONE)
	$(COMPILER) $(CFLAGS) $(INC_DIR) -c $(1) -o $(2)
endef

#---------------------------------------

#MAIN_SUPEX = supex_scco_http
#[=========>MARK<=========]#
ifeq ($(MAIN_SUPEX), supex_scco)
HANDLE_MODEL = SCCO
endif

ifeq ($(MAIN_SUPEX), supex_line)
HANDLE_MODEL = LINE
endif

ifeq ($(MAIN_SUPEX), supex_evuv)
HANDLE_MODEL = EVUV
endif

ifeq ($(MAIN_SUPEX), supex_evcoro)
HANDLE_MODEL = EVCORO
endif

#[=========>MARK<=========]#
ifeq ($(OBJECT_SCENE), OFFLINE)
ADD_OBJ += instrument.o
endif
#---------------------------------------

##基础库需要的文件
BASE_OBJ_DIR = ./obj_supex_base
BASE_OBJ = $(addprefix $(BASE_OBJ_DIR)/, \
	$(ADD_OBJ) \
	rbtree.o \
	utils.o \
	dialog.o \
	same_kill.o \
	list.o  \
	match.o \
	net_cache.o \
	hashmap.o \
	sstr.o \
	cnt_pool.o \
	pool_api.o \
	async_obj.o \
	async_api.o \
	async_evt.o \
	async_cnt.o \
	http.o \
	http_parser.o \
	mttp.o \
	redis_parse.o \
	share_evcb.o \
	tcp_api.o \
	lua_sync_link.o \
	lua_async_link.o \
	lua_scco.o \
	lua_evcoro.o \
	supex.o \
	switch_queue.o \
	zmq_wrap.o \
	)

##框架库需要的文件
FRAME_OBJ_DIR=./obj_$(MAIN_SUPEX)
FRAME_OBJ = $(addprefix $(FRAME_OBJ_DIR)/, \
	   major_def.o \
	   split_task.o \
	   split_evcb.o \
	   split_api.o \
	   smart_task.o \
	   smart_evcb.o \
	   smart_api.o \
	   swift_api.o \
	   swift_evcb.o \
	   swift_task.o \
	   crzpt_task.o \
	   crzpt_plan.o \
	   crzpt_evcb.o \
	   crzpt_api.o \
	   sniff_api.o \
	   sniff_task.o \
	   solo_api.o \
	   )

#---------------------------------------
#需要编译的目录
BASE_DIR=.

#<<<<<----------------------------------
MAJOR_DIR=$(BASE_DIR)/major
MINOR_DIR=$(BASE_DIR)/minor

#---------------------------------------
ifeq ($(MAIN_SUPEX),)
MAIN_SUPEX = supex_base
endif

ifneq ($(MAIN_SUPEX), supex_base)
OBJ = $(BASE_OBJ) $(FRAME_OBJ)
else
OBJ = $(BASE_OBJ)
endif

PJT := lib$(MAIN_SUPEX).a

#---------------------------------------
#规则标签
all: prepare $(PJT)

prepare:
	@-if [ ! -d $(BASE_OBJ_DIR) ];then mkdir $(BASE_OBJ_DIR); fi
	@-if [ ! -d $(FRAME_OBJ_DIR) ];then mkdir $(FRAME_OBJ_DIR); fi

$(PJT): $(OBJ)
	$(ECHO) $(COLOR_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_TAG)"$(PJT)"$(COLOR_NONE)
	$(AC) $@ $^
	$(ECHO) $(COLOR_TXT)"\n\t- BUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

#---------------------------------------
#编译框架中相关文件夹下的c文件
#一级目录编译
$(BASE_OBJ_DIR)/%.o : $(BASE_DIR)/%.c
	$(call compile_obj, $^, $@)

#<<<<<----------------------------------

$(FRAME_OBJ_DIR)/%.o : $(MAJOR_DIR)/%.c
	$(call compile_obj, $^, $@)

$(FRAME_OBJ_DIR)/%.o : $(MINOR_DIR)/%.c
	$(call compile_obj, $^, $@)

#--------------------------------------
#外部c文件
$(BASE_OBJ_DIR)/instrument.o : ../pvtrace/instrument.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/http_parser.o : ../lib/http-parser/http_parser.c
	$(call compile_obj, $^, $@)

#---------------------------------------
#工具程序
xctl:
	$(COMPILER) -g $(INC_DIR) $(LIB_DIR) $(BASE_DIR)/xctl.c -o $@ -lcrypto -lrt -lluajit-5.1 -ldl
	$(ECHO) $(COLOR_TXT)"\t\t- LINK\t===>\t"$(COLOR_TAG)"$(2)"$(COLOR_NONE)

sclnt: $(BASE_OBJ_DIR)/utils.o
	$(COMPILER) -g $(INC_DIR) $(LIB_DIR) $(BASE_DIR)/sclnt.c $^ -o $@ -lcrypto -lmini -lmemhook -lrt -lluajit-5.1 -lpthread -ldl
	$(ECHO) $(COLOR_TXT)"\t\t- LINK\t===>\t"$(COLOR_TAG)"$(2)"$(COLOR_NONE)

#---------------------------------------
clean:
	rm -rf $(BASE_OBJ_DIR)
	rm -rf $(FRAME_OBJ_DIR)
	rm -rf $(PJT)

distclean:
	make clean
	rm -rf ./obj_supex*
	rm -rf ./lib*.a
	rm -rf ./xctl
	rm -rf ./sclnt
