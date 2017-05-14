COLOR_NONE            = "\x1B[m"
COLOR_GRAY            = "\x1B[1;30m"
COLOR_RED             = "\x1B[1;31m"
COLOR_GREEN           = "\x1B[1;32m"
COLOR_YELLOW          = "\x1B[1;33m"
COLOR_BLUE            = "\x1B[1;34m"
COLOR_PURPLE          = "\x1B[1;35m"
COLOR_CYAN            = "\x1B[1;36m"
COLOR_WHITE           = "\x1B[1;37m"

#######
COLOR_FILE_OBJ =	$(COLOR_CYAN)
COLOR_FILE_TAG =	$(COLOR_PURPLE)

#######
BUILD_PWD = 		$(shell pwd)
BUILD_HOST =		$(shell uname | tr '[A-Z]' '[a-z]')

#######

MAIN_APP_SERV ?=	BRM
#######
BUILD_CFLAGS =		-g -Wall -O1
BUILD_LIBS =		-lluajit-5.1 -lcrypto -lpthread -lm -lstdc++ -ldl
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include -I/usr/local/include/luajit-2.0
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec

ADD_DYLIBS =
ADD_ARLIBS =		-levcs -lmini -lkv -lev -levcoro -lcjson -lscco -lcoro -lzmq
ADD_INC_DIR = 		-I$(BUILD_PWD)/../../lib/libmini/include \
			-I$(BUILD_PWD)/../../lib/libmemhook/src \
			-I$(BUILD_PWD)/../../lib/libev \
			-I$(BUILD_PWD)/../../lib/zeromq-4.2.1/include \
			-I$(BUILD_PWD)/../../lib/cJSON \
			-I$(BUILD_PWD)/../../lib/libscco/include \
			-I$(BUILD_PWD)/../../lib/libcoro/include \
			-I$(BUILD_PWD)/../../lib/libevcoro/include \
			-I$(BUILD_PWD)/../../lib/libevcs/include \
			-I$(BUILD_PWD)/../../lib/libevcs/include/engine \
			-I$(BUILD_PWD)/../../lib/libevcs/include/frame \
			-I$(BUILD_PWD)/../../lib/libkv/src \
			-I$(BUILD_PWD)/../

ADD_LIB_DIR =		-L$(BUILD_PWD)/../../lib
#######

ifeq ($(BUILD_HOST), darwin)
ECHO =			@echo
BUILD_CFLAGS +=		-D_GNU_SOURCE
else
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =				@echo -e
ADD_DYLIBS +=		-lrt
BUILD_CFLAGS +=		-D_GNU_SOURCE -export-dynamic
endif

#开启内存泄漏检测
ifeq ($(MEMHOOK), yes)
BUILD_CFLAGS += 	-DUSE_MEMHOOK \
	-export-dynamic \
	-Wl,-wrap,malloc -Wl,-wrap,calloc \
	-Wl,-wrap,realloc -Wl,-wrap,free \
	-Wl,-wrap,strdup
ADD_ARLIBS += -lmemhook
endif

#######
TARGET ?= 		$(MAIN_APP_SERV)
SRC_DIR = 		./src
OBJ_DIR = 		$(BUILD_PWD)/.obj
OBJS = 			$(addprefix $(OBJ_DIR)/, \
				data_model.o \
				parse_cfg.o \
				prg_frame.o \
				recv_data.o \
				route_data.o \
				calculate_data.o \
				except_info.o \
				ctrl_proc.o \
				main.o \
			)

#######

CC_COMPILER ?=		gcc
CC =			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_INC_DIR) $(ADD_INC_DIR) $(BUILD_OPTIMIZE)
LN = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(DYLIB_CFLAGS) $(BUILD_LIB_DIR)
AR = 			@ar -rcs


all : prepare $(TARGET)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_FILE_OBJ)"$(<:.c=.o)"$(COLOR_NONE)
	$(CC) -o $@ -c $<

$(TARGET) : $(OBJS) 
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_FILE_TAG)"$@"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $^ $(ADD_ARLIBS) -o $@ $(ADD_DYLIBS) $(BUILD_LIBS)

prepare :
	@-if [ ! -d $(OBJ_DIR) ];then mkdir -p $(OBJ_DIR); fi

clean :
	rm -rf $(OBJ_DIR)
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

distclean : clean
	rm -rf $(TARGET)
	rm -rf ./mhooktrace.log
	rm -rf ./log
	$(ECHO) $(COLOR_GREEN)"\n\tDISTCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)
	

push:
	git push origin HEAD:refs/for/master




