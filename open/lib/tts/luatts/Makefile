HOST		=$(shell uname|tr '[A-Z]' '[a-z]')
WORK_DIR	=.
SRC_DIR		=$(WORK_DIR)/src
OBJ_DIR		=$(WORK_DIR)/.obj
OUT_DIR		=$(WORK_DIR)/lib

COLOR_NONE            = "\x1B[m"
COLOR_RED             = "\x1B[0;31m"
COLOR_GREEN           = "\x1B[0;32m"
COLOR_BLUE            = "\x1B[0;34m"

INC_DIR	   ?=-I../libs
INC_DIR	   +=-I/usr/local/include/luajit-2.0
LIB_DIR    ?=-L../libs
LIB_DIR	   +=-L/usr/local/lib


ADD_LIBS	=-lusc -lluajit-5.1 -lm

OUT_LIB		=$(OUT_DIR)/luatts.so $(OUT_DIR)/data2wav.so

ifeq ($(HOST), linux)
LIBFLAGS	=-shared
ECHO 		=@echo -e
endif
ifeq ($(HOST), darwin)
LIBFLAGS	=-bundle -undefined dynamic_lookup
ECHO 		=@echo
endif

TEST_BIN =	/usr/local/bin/luajit
TEST_SHL =	./src/test.lua

CFLAGS		?=-O3
ADD_CFLAGS	+=$(CFLAGS) -fPIC

OBJ = $(addprefix $(OBJ_DIR)/, lua_tts.o data2wav.o)

all : $(OUT_LIB)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@-if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_BLUE)"$(<:.c=.o)"$(COLOR_NONE)
	gcc $(ADD_CFLAGS) $(INC_DIR) -c $< -o $@

$(OUT_LIB) : $(OBJ)
	@-if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi
	gcc $(ADD_CFLAGS) $^ $(ADD_LIBA) $(LIB_DIR) $(LIBFLAGS) -o $@ $(ADD_LIBS)
	$(ECHO) $(COLOR_GREEN)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)


clean :
	@rm -rf $(OBJ_DIR)
	@rm -rf $(OUT_LIB)
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

.PHONY: all
