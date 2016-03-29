
COLOR_NONE		="\x1B[m"
COLOR_GRAY		="\x1B[0;30m"
COLOR_RED		="\x1B[0;31m"
COLOR_GREEN		="\x1B[0;32m"
COLOR_YELLOW		="\x1B[0;33m"
COLOR_BLUE		="\x1B[0;34m"
COLOR_PURPLE		="\x1B[0;35m"
COLOR_CYAN		="\x1B[0;36m"
COLOR_WHITE		="\x1B[0;37m"

RUN_PWD = $(shell pwd)

CC_COMPILER	?= gcc
CC 			= ${CC_COMPILER} ${CC_CFLAGS} ${CC_IPATH}
LN 			= ${CC_COMPILER} ${CC_CFLAGS} ${CC_IPATH}
AR          = ar -rcs
CP          = cp -f
RM          = rm -f
MV          = mv -f
ECHO 		= echo

SLIBSUFFIXES ?= .a

DLIBSUFFIXES = ${DYLIB_SUFFIXES}
DLIBSUFFIXES ?= .so
BUILDSUFFIXES = ${BUILD_SUFFIXES}
BUILDSUFFIXES ?= .o

ifeq ($(BUILD_HOST), linux)
ECHO 		= echo -e
endif

.SUFFIXES:
.SUFFIXES: .o .c
.SUFFIXES: .o .cc
.SUFFIXES: .o .cpp

.c.o:
	@$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t$(<:.c=.o)"$(COLOR_NONE)
	@$(CC) -o $(<:.c=.o) -c $<
	@$(ECHO) $(COLOR_YELLOW)"\t\t- MOVE\t===>\t$(BUILD_OBJS)"$(COLOR_NONE)
	@$(MV) $(<:.c=.o) $(BUILD_OBJS)/
.cc.o:
	@$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t$(<:.cc=.o)"$(COLOR_NONE)
	@$(CC) -o $(<:.cc=.o) -c $<
	@$(ECHO) $(COLOR_YELLOW)"\t\t- MOVE\t===>\t$(BUILD_OBJS)"$(COLOR_NONE)
	@$(MV) $(<:.c=.o) $(BUILD_OBJS)/
.cpp.o:
	@$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t$(<:.cpp=.o)"$(COLOR_NONE)
	@$(CC) -o $(<:.cpp=.o) -c $<
	@$(ECHO) $(COLOR_YELLOW)"\t\t- MOVE\t===>\t$(BUILD_OBJS)"$(COLOR_NONE)
	@$(MV) $(<:.c=.o) $(BUILD_OBJS)/





all: setup

#$(foreach obj, $(OBJS), $(obj).o)
build: load $(OBJS)

bin:
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t===>\t$(RUN_PWD)"$(COLOR_NONE)
	@$(ECHO) $(COLOR_GREEN)"\t\t- LINK\t===>\t$(TARGET)"$(COLOR_NONE)
	$(LN) $(OBJS) $(AR_OBJS) $(CC_LPATH) $(SHARE_OBJS) -o $(TARGET)
	@$(ECHO) $(COLOR_YELLOW) "\t\t- MOVE\t===>\t$(BUILD_BIN)"$(COLOR_NONE)
	@$(MV) $(TARGET) $(BUILD_BIN)

lib:
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t$(RUN_PWD)"$(COLOR_NONE)
	@$(ECHO) $(COLOR_GREEN)"\t\t- ARCHIVE\t===>\t$(TARGET)$(SLIBSUFFIXES)"$(COLOR_NONE)
	$(AR) $(TARGET)$(SLIBSUFFIXES) $(OBJS)
	@$(ECHO) $(COLOR_GREEN)"\t\t- DYNAMIC\t===>\t$(TARGET)$(DLIBSUFFIXES)"$(COLOR_NONE)
	$(LN) $(OBJS) $(AR_OBJS) $(CC_LPATH) $(SHARE_OBJS) $(DYLIB_FLAGS) -o $(TARGET)$(DLIBSUFFIXES)
	@$(ECHO) $(COLOR_GREEN)"\t\t- MOVE\t===>\t$(BUILD_LIB)"$(COLOR_NONE)
	@$(MV) $(TARGET)$(DLIBSUFFIXES) $(TARGET)$(SLIBSUFFIXES) $(BUILD_LIB)

load:
	@$(ECHO) $(COLOR_YELLOW)"\t\t- BUILD\t$(shell basename $(RUN_PWD))"$(COLOR_NONE)
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t$(RUN_PWD)"$(COLOR_NONE)
ifeq ($(OBJS), )
	@$(ECHO) $(COLOR_RED)"\t\tERROR Operate!!!\n\t\tPlease use like:"
	@$(ECHO) "\t\t  make -f func.mk OBJS=(main.c other.c) or (main.o other.o)"$(COLOR_NONE)
endif



clean: $(OBJS)
	@$(ECHO) $(COLOR_RED)"\t\t- CLEAN\t$(shell basename $(RUN_PWD))"$(COLOR_NONE)
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t$(shell dirname $(RUN_PWD))"$(COLOR_NONE)
	@$(ECHO) $(COLOR_RED) "\t\t- RM\t===>\t*$(BUILDSUFFIXES)"$(COLOR_NONE)
	@$(RM) $(OBJS)

cleanobjs:
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t===>\t$(BUILD_PWD)"$(COLOR_NONE)
	@$(ECHO) $(COLOR_RED) "\t\t- RM\t===>\t*$(BUILDSUFFIXES)"$(COLOR_NONE)
	@$(RM) *$(BUILDSUFFIXES)


cleanbin:$(TARGET)
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t===>\t$(RUN_PWD)"$(COLOR_NONE)
	@$(ECHO) $(COLOR_RED)"\t\t- RM\t===>\t$(TARGET)"$(COLOR_NONE)
	@$(RM) $(TARGET)

cleanlib: $(TARGET)$(SLIBSUFFIXES) $(TARGET)$(DLIBSUFFIXES)
	@$(ECHO) $(COLOR_YELLOW)"\t\t- DIR\t===>\t$(RUN_PWD)"$(COLOR_NONE)
	@$(ECHO) $(COLOR_RED)"\t\t- RM\t===>\t$(TARGET)$(SLIBSUFFIXES) & $(TARGET)$(DLIBSUFFIXES)"$(COLOR_NONE)
	@$(RM) $(TARGET)$(SLIBSUFFIXES) $(TARGET)$(DLIBSUFFIXES)

.PHONY: cleanlib cleanbin cleanobjs clean load
