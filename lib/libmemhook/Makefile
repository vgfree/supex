COLOR_NONE            = "\x1B[m"
COLOR_GRAY            = "\x1B[1;30m"
COLOR_RED             = "\x1B[1;31m"
COLOR_GREEN           = "\x1B[1;32m"
COLOR_YELLOW          = "\x1B[1;33m"
COLOR_BLUE            = "\x1B[1;34m"
COLOR_PURPLE          = "\x1B[1;35m"
COLOR_CYAN            = "\x1B[1;36m"
COLOR_WHITE           = "\x1B[1;37m"

COLOR_FILE_OBJ =	$(COLOR_CYAN)
COLOR_FILE_TAG =	$(COLOR_YELLOW)
COLOR_FILE_TXT = 	$(COLOR_GREEN)
#######

BUILD_PWD = 		$(shell pwd)
BUILD_HOST =		$(shell uname | tr '[A-Z]' '[a-z]')

TOBJ =			libmemhook
TOBJ_MAJOR ?= 		1
TOBJ_MINOR ?= 		0

TOBJ_ARNAME =		$(TOBJ).a

SOBJ_DIR =		.objs
TOBJ_DIR =		lib

#######

BUILD_CFLAGS =		-fPIC -Wall
BUILD_LIBS =		-lpthread -lm
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include -I../libmini/include
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec -L../

#######

ifeq ($(BUILD_HOST), darwin)

ECHO =			@echo
TOBJ_DLSUFFIX =		dylib
TOBJ_NAME = 		$(TOBJ).$(TOBJ_DLSUFFIX)
TOBJ_DLNAME =		$(TOBJ_NAME)
DYLIB_CFLAGS =		-dynamiclib

BUILD_INC_DIR +=	-I/usr/local/include/jemalloc

else
BUILD_CFLAGS += -D_GNU_SOURCE -DTRACE -export-dynamic
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =			@echo -e
TOBJ_DLSUFFIX =		so
TOBJ_NAME = 		$(TOBJ).$(TOBJ_DLSUFFIX)
TOBJ_DLNAME =		$(TOBJ_NAME).$(TOBJ_MAJOR).$(TOBJ_MINOR)
TOBJ_SONAME = 		$(TOBJ_NAME).$(TOBJ_MAJOR)
DYLIB_CFLAGS =		-shared -Wl,-soname,$(TOBJ_SONAME)
endif



#######

CC_COMPILER ?=		@gcc
CC =			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(BUILD_INC_DIR) $(BUILD_OPTIMIZE)
LN = 			$(CC_COMPILER) $(BUILD_CFLAGS) $(CFLAGS) $(DYLIB_CFLAGS) $(BUILD_LIB_DIR)
AR = 			@ar -rcs

#######

all : prepare $(TOBJ)

prepare :
	@-if [ ! -d $(SOBJ_DIR) ];then mkdir $(SOBJ_DIR); fi
	@-if [ ! -d $(TOBJ_DIR) ];then mkdir $(TOBJ_DIR); fi

#######以下是你需要修改添加的#######

SFILE_BASE_DIR = 	src

#######
SOBJ = $(addprefix $(SOBJ_DIR)/, \
	malloc_hook.o \
	node_manager.o \
	)

#######

$(SOBJ_DIR)/%.o : $(SFILE_BASE_DIR)/%.c
	$(ECHO) $(COLOR_FILE_TXT)"\t\t- COMPILE\t===>\t"$(COLOR_FILE_OBJ)"$(<:.c=.o)"$(COLOR_NONE)
	$(CC) -c $< -o $@


INSTALL_HEAD_DIR = include
INSTALL_HEAD_FILE = 	@-rm -rf $(INSTALL_HEAD_DIR); \
			cp -rf $(SFILE_BASE_DIR) $(INSTALL_HEAD_DIR);	\
			find $(INSTALL_HEAD_DIR) -name "*.c" -exec rm {} \;

ifeq ($(BUILD_HOST), linux)
INSTALL_DYLIB_FILE = 	@-ln -sf $(TOBJ_DLNAME) $(TOBJ_DIR)/$(TOBJ_SONAME); \
			ln -sf $(TOBJ_SONAME) $(TOBJ_DIR)/$(TOBJ_NAME)
endif
#######以上是你需要修改添加的#######


$(TOBJ) : $(SOBJ)
	$(ECHO) $(COLOR_FILE_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_FILE_TAG)"$(TOBJ_ARNAME)"$(COLOR_NONE)
	$(AR) $(TOBJ_DIR)/$(TOBJ_ARNAME) $^
	#$(ECHO) $(COLOR_FILE_TXT)"\t\t- DYNAMIC\t===>\t"$(COLOR_FILE_TAG)"$(TOBJ_NAME)"$(COLOR_NONE)
	#$(LN) -o $(TOBJ_DIR)/$(TOBJ_DLNAME) $^ $(BUILD_LIBS)
	$(INSTALL_HEAD_FILE)
	#$(INSTALL_DYLIB_FILE)
	$(ECHO) $(COLOR_FILE_TXT)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_FILE_TXT)" <<< COMPLETE"$(COLOR_NONE)


clean :
	@-rm -rf $(SOBJ_DIR)
	$(ECHO) $(COLOR_FILE_TXT)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_FILE_TXT)" <<< COMPLETE"$(COLOR_NONE)

distclean : clean
	@-rm -rf $(TOBJ_DIR)
	@-rm -rf $(INSTALL_HEAD_DIR)
	$(ECHO) $(COLOR_FILE_TXT)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_FILE_TXT)" <<< COMPLETE"$(COLOR_NONE)

install : $(TOBJ)
	$(ECHO) $(COLOR_FILE_TXT)"\t\t- INSTALL\t===>\t$<"$(COLOR_NONE)
ifeq ($(BUILD_HOST), darwin)
	
else
	
endif
