COLOR_NONE            = "\x1B[m"
COLOR_GRAY            = "\x1B[0;30m"
COLOR_RED             = "\x1B[0;31m"
COLOR_GREEN           = "\x1B[0;32m"
COLOR_YELLOW          = "\x1B[0;33m"
COLOR_BLUE            = "\x1B[0;34m"
COLOR_PURPLE          = "\x1B[0;35m"
COLOR_CYAN            = "\x1B[0;36m"
COLOR_WHITE           = "\x1B[0;37m"

WORK_DIR        =$(shell pwd)
BUILD_HOST =	$(shell uname | tr '[A-Z]' '[a-z]')
OBJ             =test.o common_test.o
LIBA 		=$(WORK_DIR)/../../libmemhook.a $(WORK_DIR)/../../libmini.a
LIB 		= -lpthread -ldl 
INC			=-I$(WORK_DIR)/../src -I$(WORK_DIR)/../../libmini/include
LIBINC 		=-L/usr/local/lib
ifeq ($(BUILD_HOST), linux)
ECHO =			@echo -e
CFLAGS 		= -export-dynamic -Wl,-wrap,malloc -Wl,-wrap,calloc -Wl,-wrap,realloc -Wl,-wrap,free
else
LIB 		+= -ljemalloc
INC		+= -I/usr/local/include/jemalloc
ECHO =			@echo
endif

all:clean test

%.o:%.c
	@gcc -c $< $(INC)

test:$(OBJ)
	@-mkdir -p $(WORK_DIR)/../bin
	@gcc $(CFLAGS) $(OBJ) $(LIBA) $(LIBINC) -o a.out $(LIB)
	@-mv a.out $(WORK_DIR)/../bin/
	$(ECHO) $(COLOR_GREEN)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

clean:
	@-rm -f $(WORK_DIR)/*.o
	@-rm -rf $(WORK_DIR)/../bin
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)
