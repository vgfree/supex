INC_DIR=-I/usr/local/include/luajit-2.0 \
	-I../lib/libev \
	-I../lib/http-parser \
	-I../lib/redis-parser \
	-I../lib/libevcoro/src \
	-I../lib/libmini/src

LIB_DIR=-L/usr/local/lib -L../lib
LIBS=-lluajit-5.1 -lpthread -lrt -lm

#---------------------------------------
ECHO = @echo -e
COMPILER = gcc
AC = @ar -rcs

COLOR_OBJ =	"\x1B[0;36m"
COLOR_TAG =	"\x1B[1;33m"
COLOR_TXT = "\x1B[1;32m"
COLOR_NONE = "\x1B[m"
COLOR_RED = "\x1B[1;31m"

define compile_obj
	$(ECHO) $(COLOR_TXT)"\t\t- COMPILE\t===>\t"$(COLOR_OBJ)"$(1:.c=.o)"$(COLOR_NONE)
	$(COMPILER) $(CFLAGS) $(INC_DIR) -c $(1) -o $(2)
endef

#---------------------------------------
#外部文件
ADD_OBJ = http_parser.o redis_parser.o
#---------------------------------------

##基础库需要的文件
BASE_OBJ_DIR = ./obj
BASE_OBJ = $(addprefix $(BASE_OBJ_DIR)/, \
	$(ADD_OBJ) \
	utils.o \
	cache.o \
	tcp_io.o \
	tcp_base.o \
	http_parse.o \
	http_reqresp.o \
	http_string.o \
	redis_parse.o \
	redis_reqresp.o \
	redis_string.o \
	lj_http.o \
	netpkg_statistic.o \
	pool.o \
	connection_pool.o \
	)

##框架库需要的文件

#---------------------------------------
#需要编译的目录
BASE_DIR=.

#---- 一级目录
CACHE_DIR=$(BASE_DIR)/cache
TCP_API_DIR=$(BASE_DIR)/tcp_api
LUA_EXPAND_DIR=$(BASE_DIR)/lua_expand
POOL_API_DIR=$(BASE_DIR)/pool_api
NETPKG_STAT_DIR=$(BASE_DIR)/netpkg_statistic

#---- 二级目录
CONN_POOL_DIR=$(TCP_API_DIR)/connection_pool
HTTP_API_DIR=$(TCP_API_DIR)/http_api
REDIS_API_DIR=$(TCP_API_DIR)/redis_api
ASYNC_GRPREQ_DIR=$(TCP_API_DIR)/async_grpreq
#---------------------------------------
ifeq ($(LIBNAME),)
LIBNAME = frame
endif

PJT := lib$(LIBNAME).a

#---------------------------------------
#规则标签
all: prepare $(PJT)

prepare:
	@-if [ ! -d $(BASE_OBJ_DIR) ];then mkdir $(BASE_OBJ_DIR); fi
	#@-if [ ! -d $(FRAME_OBJ_DIR) ];then mkdir $(FRAME_OBJ_DIR); fi

$(PJT): $(BASE_OBJ)
	$(ECHO) $(COLOR_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_TAG)"$(PJT)"$(COLOR_NONE)
	$(AC) $@ $^
	$(ECHO) $(COLOR_TXT)"\n\t- BUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

#---------------------------------------
#编译框架中相关文件夹下的c文件
#一级目录编译
$(BASE_OBJ_DIR)/%.o : $(BASE_DIR)/%.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/%.o : $(TCP_API_DIR)/%.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/%.o : $(CACHE_DIR)/%.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/%.o : $(LUA_EXPAND_DIR)/%.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/%.o : $(POOL_API_DIR)/%.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/%.o : $(NETPKG_STAT_DIR)/%.c
	$(call compile_obj, $^, $@)

#---------------------------------------
#二级目录编译
$(BASE_OBJ_DIR)/%.o : $(CONN_POOL_DIR)/%.c
	$(call compile_obj, $^, $@)
$(BASE_OBJ_DIR)/%.o : $(HTTP_API_DIR)/%.c
	$(call compile_obj, $^, $@)
$(BASE_OBJ_DIR)/%.o : $(REDIS_API_DIR)/%.c
	$(call compile_obj, $^, $@)

#--------------------------------------
#外部c文件
$(BASE_OBJ_DIR)/redis_parser.o : ../lib/redis-parser/redis_parser.c
	$(call compile_obj, $^, $@)

$(BASE_OBJ_DIR)/http_parser.o : ../lib/http-parser/http_parser.c
	$(call compile_obj, $^, $@)

#---------------------------------------
#工具程序

#---------------------------------------
clean:
	rm -rf $(BASE_OBJ_DIR)

distclean:
	make clean
	rm -rf ./lib*.a
