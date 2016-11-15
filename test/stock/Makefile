INC_DIR=-I./src -I/usr/local/include/luajit-2.0
LIB_DIR=-L/usr/local/lib
CFLAGS =-fPIC -Wall -fdiagnostics-color=auto
LFLAGS =-shared

LIBS=-lpthread
#---------------------------------------
ECHO = @echo -e
COMPILER = gcc -g
AC = @ar -rcs

COLOR_NONE	=	"\x1B[m"
COLOR_OBJ	=	"\x1B[0;36m"
COLOR_TAG	=	"\x1B[0;33m"
COLOR_TXT	=	"\x1B[0;32m"
COLOR_RED	=	"\x1B[0;31m"

define compile_obj
	$(ECHO) $(COLOR_TXT)"\t\t- COMPILE\t===>\t"$(COLOR_OBJ)"$(1:.c=.o)"$(COLOR_NONE)
	$(COMPILER) $(CFLAGS) $(INC_DIR) -c $(1) -o $(2)
endef

#---------------------------------------
#需要编译的目录
BASE_SRC_DIR=.

##需要编译的文件
BASE_OBJ_DIR = ./obj
#---------------------------------------

OBJ = $(addprefix $(BASE_OBJ_DIR)/, \
	stock.o \
	stock_shm.o \
	)
#---------------------------------------
#规则标签
all: prepare stock.so libstock.a example

prepare:
	@-if [ ! -d $(BASE_OBJ_DIR) ];then mkdir $(BASE_OBJ_DIR); fi

example:
	$(COMPILER) $(CFLAGS) $(INC_DIR) test/test.c libstock.a -lluajit-5.1 -o $@

stock.so: $(OBJ)
	$(ECHO) $(COLOR_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_TAG)"stock.so"$(COLOR_NONE)
	@$(COMPILER) $(CFLAGS) $(INC_DIR) $^ $(LIB_DIR) $(LFLAGS) -o $@ $(LIBS)
	$(ECHO) $(COLOR_TXT)"\n\t- BUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

libstock.a: $(OBJ)
	$(ECHO) $(COLOR_TXT)"\t\t- ARCHIVE\t===>\t"$(COLOR_TAG)"libstock.a"$(COLOR_NONE)
	$(AC) $@ $^
	$(ECHO) $(COLOR_TXT)"\n\t- BUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_TXT)" <<< COMPLETE"$(COLOR_NONE)

#---------------------------------------
#编译框架中相关文件夹下的c文件
$(BASE_OBJ_DIR)/%.o : $(BASE_SRC_DIR)/src/%.c
	$(call compile_obj, $^, $@)
#---------------------------------------
clean:
	rm -rf $(BASE_OBJ_DIR)
	rm -rf stock.so
	rm -rf libstock.a
	rm -rf example


debug:
	gdb --args luajit init.lua


.PHONY: prepare stock.so libstock.a example
