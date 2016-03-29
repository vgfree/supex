COLOR_NONE            = "\x1B[m"
COLOR_GRAY            = "\x1B[0;30m"
COLOR_RED             = "\x1B[0;31m"
COLOR_GREEN           = "\x1B[0;32m"
COLOR_YELLOW          = "\x1B[0;33m"
COLOR_BLUE            = "\x1B[0;34m"
COLOR_PURPLE          = "\x1B[0;35m"
COLOR_CYAN            = "\x1B[0;36m"
COLOR_WHITE           = "\x1B[0;37m"

#######

BUILD_PWD = 		$(shell pwd)
BUILD_HOST =		$(shell uname | tr '[A-Z]' '[a-z]')

#######

BUILD_CFLAGS =		-Wall
BUILD_LIBS =		-lpthread -lm -ldl
BUILD_INC_DIR = 	-I/usr/include -I/usr/local/include
BUILD_LIB_DIR =		-L/usr/local/lib -L/usr/lib -L/usr/libexec

ADD_DYLIBS =
#ADD_ARLIBS =		-lmini
#ADD_INC_DIR = 		-I$(BUILD_PWD)/../../lib/libmini/include
#ADD_LIB_DIR =		-L$(BUILD_PWD)/../../lib -L$(BUILD_PWD)/../../lib/libmini/lib

#######

ifeq ($(BUILD_HOST), darwin)
ECHO =				@echo
else
BUILD_LIB_DIR +=	-L/usr/local/lib64 -L/usr/lib64
ECHO =				@echo -e
#BUILD_OPTIMIZE =	-fprofile-arcs
#ADD_DYLIBS += 		-lgcov -lrt
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
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t$(<:.c=.o)"$(COLOR_NONE)
	$(CC) -o $(<:.c=.o) -c $<

#######
BIN_DIR = bin
BIN = $(addprefix $(BIN_DIR)/, \
	base_test \
	loop_test \
	)

ifeq ($(BUILD_HOST), linux)
#BIN += $(addprefix $(BIN_DIR)/, \
#	)
endif


all : clean prepare $(BIN)

prepare :
	@-if [ ! -d $(BIN_DIR) ];then mkdir $(BIN_DIR); fi
	$(CC_COMPILER) -v

clean :
	@-find . -name "*.o" -exec rm {} \;
	@-rm -rf $(BIN_DIR)
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

#######以下是你需要修改添加的#######

$(BIN_DIR)/base_test : base_test/main.o
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_BLUE)"$@"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $< $(ADD_ARLIBS) -o $@ $(ADD_DYLIBS) $(BUILD_LIBS)

$(BIN_DIR)/loop_test : loop_test/main.o
	$(ECHO) $(COLOR_GREEN)"\t\t-   BUILD\t===>\t"$(COLOR_BLUE)"$@"$(COLOR_NONE)
	$(LN) $(ADD_LIB_DIR) $< $(ADD_ARLIBS) -o $@ $(ADD_DYLIBS) $(BUILD_LIBS)








#######
