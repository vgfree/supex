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

BUILD_CFLAGS =		-g -Wall -O1
BUILD_LIBS =		-lluajit-5.1 -lpthread -lm -lstdc++ -lrt -ldl
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include -I/usr/local/include/luajit-2.0 
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec 

ADD_DYLIBS =
ADD_ARLIBS =		-levcs -lmini -lkv -lev -levcoro -lcjson -lscco -lcoro -lzmq
ADD_INC_DIR = 		-I$(BUILD_PWD)/../../lib/libmini/include \
			-I$(BUILD_PWD)/../../lib/libmemhook/src \
			-I$(BUILD_PWD)/../../lib/libev \
			-I$(BUILD_PWD)/../../lib/zeromq-4.1.4/include \
			-I$(BUILD_PWD)/../../lib/cJSON \
			-I$(BUILD_PWD)/../../lib/libscco/include \
			-I$(BUILD_PWD)/../../lib/libcoro/include \
			-I$(BUILD_PWD)/../../lib/libevcoro/include \
			-I$(BUILD_PWD)/../../lib/libevcs/include/engine \
			-I$(BUILD_PWD)/../../lib/libevcs/include/frame \
			-I$(BUILD_PWD)/../

ADD_LIB_DIR =		-L$(BUILD_PWD)/../../lib

#######
BUILD_CFLAGS +=		$(EXPORT_CFLAGS)
BUILD_LIBS +=		$(EXPORT_LIBA)


#######

ifeq ($(BUILD_HOST), darwin)
ECHO =				@echo
else
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =				@echo -e
endif

#######

MAIN_APP_SERV ?=	damR
APP ?= 			$(MAIN_APP_SERV)

ifeq ($(APP), dam)

endif

#######

CC_COMPILER ?=		@gcc
CC =			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_INC_DIR) $(ADD_INC_DIR) $(BUILD_OPTIMIZE)
LN = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(DYLIB_CFLAGS) $(BUILD_LIB_DIR)
AR = 			@ar -rcs

#######
OBJ_DIR = bin
OBJ = $(addprefix $(OBJ_DIR)/, \
	connection_pool.o \
	ctrl_loop.o \
	data_model.o \
	data_queue.o \
	load_cfg.o \
	main.o \
	prg_frame.o \
	recv_data4zmq.o \
	send_data4zmq.o \
	zmq_wrap.o \
	)

#######
DATA_DIR = data
CLEANFILES = mhooktrace.log

#######

all : prepare $(APP)

prepare :
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi

clean :
	@-find . -name "*.o" -exec rm {} \;
	@-rm -f $(OBJ_DIR)/*
	@rm -f $(CLEANFILES)
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

distclean : clean
	@-rm -rf $(OBJ_DIR)
	@-rm -rf $(APP)
	@-rm -rf $(DATA_DIR)
	$(ECHO) $(COLOR_GREEN)"\n\tDISTCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

$(APP) : $(OBJ)
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_FILE_TAG)"$@"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $^ $(ADD_ARLIBS) -o $@ $(ADD_DYLIBS) $(BUILD_LIBS)

$(OBJ_DIR)/%.o : src/%.c
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_FILE_OBJ)"$@"$(COLOR_NONE)
	$(CC) -o $@ -c $<
#######
$(OBJ_DIR)/zmq_wrap.o : ../zmq_wrap.c
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi
	$(CC) -o $@ -c $<
