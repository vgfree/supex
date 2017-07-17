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
COLOR_OBJ =	$(COLOR_CYAN)
COLOR_TAG =	$(COLOR_YELLOW)
COLOR_TXT = 	$(COLOR_GREEN)

#######
BUILD_PWD = 		$(shell pwd)
BUILD_HOST =		$(shell uname | tr '[A-Z]' '[a-z]')

BUILD_CFLAGS =		-fPIC -Wall -Wno-unused-function -Wno-unused-variable -g
BUILD_LIBS =		-lpthread -lm -lcomm
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include -I../liblog/
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec -L$(BUILD_PWD)/lib

#######
CC_COMPILER ?=		gcc
LN = 			@$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(DYLIB_CFLAGS)
CC =			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_OPTIMIZE) $(BUILD_INC_DIR) $(BUILD_LIB_DIR)
AR = 			@ar -rcs

#######
TARGET =		libcomm
TARGET_MAJOR ?=		1
TARGET_MINOR ?= 	0
TARGET_ARNAME =		$(TARGET).a

SRC =			$(wildcard $(BUILD_PWD)/*.c) $(wildcard $(BUILD_PWD)/mfptp_protocol/*.c)
OBJ =			$(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC))) 
OBJ_DIR	=		objs
TARGET_DIR =		lib

vpath %.c ./mfptp_protocol	#指定编译需要查找.c文件的目录

#######
ifeq ($(BUILD_HOST), darwin)

ECHO =			@echo
TARGET_DLSUFFIX =	dylib
TARGET_NAME = 		$(TARGET).$(TARGET_DLSUFFIX)
TARGET_DLNAME =		$(TARGET_NAME)
DYLIB_CFLAGS =		-dynamiclib
else
BUILD_CFLAGS += -D_GNU_SOURCE -export-dynamic
BUILD_LIB_DIR += -L/usr/local/lib64 -L/usr/lib64

ECHO =			@echo -e
TARGET_DLSUFFIX =	so
TARGET_NAME = 		$(TARGET).$(TARGET_DLSUFFIX)
TARGET_DLNAME =		$(TARGET_NAME).$(TARGET_MAJOR).$(TARGET_MINOR)
TARGET_SONAME = 	$(TARGET_NAME).$(TARGET_MAJOR)
DYLIB_CFLAGS =		-shared -Wl,-soname,$(TARGET_SONAME)
#BUILD_OPTIMIZE =	-fprofile-arcs
endif

#######
ifeq ($(BUILD_HOST), linux)
INSTALL_DYLIB_FILE = 	@-ln -sf $(TARGET_DLNAME) $(TARGET_DIR)/$(TARGET_SONAME); \
			ln -sf $(TARGET_SONAME) $(TARGET_DIR)/$(TARGET_NAME)
else
INSTALL_DYLIB_FILE =	@echo $(COLOR_RED)"Not support multi-version of library"$(COLOR_NONE)
endif

#######
define compile_obj
	$(ECHO) $(COLOR_TXT)"\t\t- COMPILE\t===>\t"$(COLOR_OBJ)"$(1:.c=.o)"$(COLOR_NONE)
	@$(CC) -c $(1) -o $(2)
endef

#######
ALL:
	#$(CC) $(BUILD_PWD)/test/timer_test.c -o $(BUILD_PWD)/test/timer_test $(BUILD_LIBS)
	#$(CC) $(BUILD_PWD)/test/list_test.c -o $(BUILD_PWD)/test/list_test $(BUILD_LIBS)
	$(CC) $(BUILD_PWD)/test/mfptp_test.c -o $(BUILD_PWD)/test/mfptp_test $(BUILD_LIBS)
	$(CC) $(BUILD_PWD)/test/server.c -o $(BUILD_PWD)/test/server $(BUILD_LIBS)
	$(CC) $(BUILD_PWD)/test/client.c -o $(BUILD_PWD)/test/client $(BUILD_LIBS)

lib: prepare  $(TARGET)

prepare:
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi
	@-if [ ! -d $(TARGET_DIR) ];then mkdir $(TARGET_DIR); fi

$(OBJ_DIR)/%.o: %.c
	$(call compile_obj, $<, $@)


$(TARGET) : $(OBJ)
	$(ECHO) $(COLOR_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_TAG)"$(TARGET_ARNAME)"$(COLOR_NONE)
	$(AR) $(TARGET_DIR)/$(TARGET_ARNAME) $^
	$(ECHO) $(COLOR_TXT)"\t\t- DYNAMIC\t===>\t"$(COLOR_TAG)"$(TARGET_NAME)"$(COLOR_NONE)
	$(LN) -o $(TARGET_DIR)/$(TARGET_DLNAME) $^ 
	$(INSTALL_HEAD_FILE)
	$(INSTALL_DYLIB_FILE)
	$(ECHO) $(COLOR_TXT)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

clean :
	@-rm -rf $(OBJ_DIR)
	$(ECHO) $(COLOR_TXT)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

distclean : clean
	@-rm -rf $(TARGET_DIR)
	@-rm -rf $(INSTALL_HEAD_DIR)
	@-rm -rf $(BUILD_PWD)/test/client
	@-rm -rf $(BUILD_PWD)/test/server
	@-rm -rf $(BUILD_PWD)/test/list_test
	@-rm -rf $(BUILD_PWD)/test/timer_test
	@-rm -rf $(BUILD_PWD)/test/mfptp_test
	@-rm -rf $(BUILD_PWD)/core*
	$(ECHO) $(COLOR_TXT)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

install : $(TARGET)
	$(ECHO) $(COLOR_TXT)"\t\t- INSTALL\t===>\t$<"$(COLOR_NONE)
