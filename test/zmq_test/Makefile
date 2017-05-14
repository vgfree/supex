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

BUILD_CFLAGS =		-Wall
BUILD_LIBS =		-lpthread -lm -lstdc++ -ldl
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec

ADD_DYLIBS =
ADD_ARLIBS =		-levcs -lmini -lzmq -lev -lcjson
ADD_INC_DIR = 		-I$(BUILD_PWD)/../../lib/libmini/include \
			-I$(BUILD_PWD)/../../lib/zeromq-4.2.1/include \
			-I$(BUILD_PWD)/../../lib/libev \
			-I$(BUILD_PWD)/../../lib/cJSON \
			-I$(BUILD_PWD)/../../engine

ADD_LIB_DIR =		-L$(BUILD_PWD)/../../lib -L$(BUILD_PWD)/../../engine

#######

ifeq ($(BUILD_HOST), darwin)
ECHO =				@echo
else
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =				@echo -e
ADD_DYLIBS +=		-lrt
BUILD_CFLAGS +=		-D_GNU_SOURCE -export-dynamic
endif

#######

CC_COMPILER ?=		@gcc
CC =			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_INC_DIR) $(ADD_INC_DIR) $(BUILD_OPTIMIZE)
LN = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(DYLIB_CFLAGS) $(BUILD_LIB_DIR)
AR = 			@ar -rcs

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_FILE_OBJ)"$(<:.c=.o)"$(COLOR_NONE)
	$(CC) -o $(<:.c=.o) -c $<

#######
BIN_DIR = bin
BIN = $(addprefix $(BIN_DIR)/, \
	pull_serv_test \
	push_clnt_test \
	pull_clnt_test \
	push_serv_test \
	zmq_and_libev \
	dam_client_test \
	libev_test \
	request_client_test \
	response_server_test \
	response_client_test \
	sub_client_test \
	pub_server_test \
	req_resp_agent_test \
	pub_sub_agent_test \
	poll_nonblock_push_test \
	poll_nonblock_pull_test \
	)

# ifeq ($(BUILD_HOST), linux)
# BIN += $(addprefix $(BIN_DIR)/, \
# 	)
# endif

define compile_link_obj
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_FILE_TAG)"$(2)"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $(1) $(ADD_ARLIBS) -o $(2) $(ADD_DYLIBS) $(BUILD_LIBS)
endef


all : prepare $(BIN)

prepare :
	@-if [ ! -d $(BIN_DIR) ];then mkdir $(BIN_DIR); fi

clean :
	@-find . -name "*.o" -exec rm {} \;
	@-rm -f $(BIN_DIR)/*
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

distclean : clean
	@-rm -rf $(BIN_DIR)
	$(ECHO) $(COLOR_GREEN)"\n\tDISTCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

#######以下是你需要修改添加的#######

$(BIN_DIR)/pull_serv_test : pull_serv_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/push_clnt_test : push_clnt_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/pull_clnt_test : pull_clnt_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/push_serv_test : push_serv_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/zmq_and_libev : zmq_and_libev/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/dam_client_test : dam_client_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/libev_test : libev_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/request_client_test : request_client_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/response_server_test : response_server_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/response_client_test : response_client_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/sub_client_test : sub_client_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/pub_server_test : pub_server_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/req_resp_agent_test : req_resp_agent_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/pub_sub_agent_test : pub_sub_agent_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/poll_nonblock_push_test : poll_nonblock_push_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/poll_nonblock_pull_test : poll_nonblock_pull_test/main.o
	$(call compile_link_obj, $<, $@)
#######
