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
BUILD_LIBS =		-lpthread -lm -ldl
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec

ADD_DYLIBS =
ADD_ARLIBS =		-lmini
ADD_INC_DIR = 		-I$(BUILD_PWD)/../../lib/libmini/include
ADD_LIB_DIR =		-L$(BUILD_PWD)/../../lib -L$(BUILD_PWD)/../../lib/libmini/lib

#######

ifeq ($(BUILD_HOST), darwin)
ECHO =				@echo
else
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =				@echo -e
# BUILD_OPTIMIZE =	-fprofile-arcs
BUILD_INC_DIR +=	-I/usr/local/include/luajit-2.0
ADD_INC_DIR +=		-I$(BUILD_PWD)/../../lib/libkv/release
ADD_LIB_DIR +=		-L$(BUILD_PWD)/../../lib/libkv/release
# ADD_DYLIBS += 		-lgcov -lrt
ADD_ARLIBS += 		
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
	utils_test \
	time_test \
	squeue_test \
	slist_test \
	shm_queue_test \
	file_queue_test \
	log_test \
	expand_slog \
	mmap_test \
	process_lock_test \
	thread_lock_test \
	futex_test \
	bit_test \
	socket_test \
	atomic_list \
	tcp_client_test \
	tcp_server_test \
	dstack_test \
	dqueue_test \
	dhash_test \
	except_test \
	signal_test \
	)

ifeq ($(BUILD_HOST), linux)
BIN += $(addprefix $(BIN_DIR)/, \
	ipc_test \
	coroutine_test \
	coroutine_simple \
	)
endif

define compile_link_obj
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_FILE_TAG)"$(2)"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $(1) -o $(2) $(ADD_ARLIBS) $(ADD_DYLIBS) $(BUILD_LIBS)
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

$(BIN_DIR)/utils_test : utils_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/time_test : time_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/squeue_test : squeue_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/slist_test : slist_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/shm_queue_test : shm_queue_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/file_queue_test : file_queue_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/log_test : log_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/expand_slog : expand_slog/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/ipc_test : ipc_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/mmap_test : mmap_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/process_lock_test : process_lock_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/coroutine_test : coroutine_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/coroutine_simple : coroutine_simple/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/thread_lock_test : thread_lock_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/futex_test : futex_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/bit_test : bit_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/socket_test : socket_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/lua_multi_vm : lua_multi_vm/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/atomic_list : atomic_list/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/tcp_server_test : tcp_server_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/tcp_client_test : tcp_client_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/dstack_test : dstack_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/dqueue_test : dqueue_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/dhash_test : dhash_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/except_test : except_test/main.o
	$(call compile_link_obj, $<, $@)

$(BIN_DIR)/signal_test : signal_test/main.o
	$(call compile_link_obj, $<, $@)

#######
