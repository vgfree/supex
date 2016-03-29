HOST		=$(shell uname|tr '[A-Z]' '[a-z]')
# 当前目录
WORK_DIR	=.
# 工作目录
SRC_DIR		=$(WORK_DIR)/src
# 目标文件目录
OBJ_DIR		=$(WORK_DIR)/.obj
# 输出文件目录
OUT_DIR		=$(WORK_DIR)/lib

COLOR_NONE            = "\x1B[m"
COLOR_RED             = "\x1B[0;31m"
COLOR_GREEN           = "\x1B[0;32m"
COLOR_BLUE            = "\x1B[0;34m"

# include 文件目录 是上级目录的libs
INC_DIR	   ?=-I../FLIF-master/src
# 追加include 文件目录
INC_DIR	   +=-I/usr/local/include/luajit-2.0
INC_DIR    +=-I../
# 库文件目录 上级目录的libs
LIB_DIR    ?=-L../
# 追加库文件目录
LIB_DIR	   +=-L/usr/local/lib

# 添加库文件
ADD_LIBS	=-lluajit-5.1 -lm -lpng16 -lflif_interface
# 共享函数库输出目录
OUT_LIB		=$(OUT_DIR)/luaflif.so

# 条件变量语句，HOST为linux时执行命令
ifeq ($(HOST), linux)
LIBFLAGS	=-shared
ECHO 		=@echo -e
endif
# MacOS时执行命令
ifeq ($(HOST), darwin)
LIBFLAGS	=-bundle -undefined dynamic_lookup
ECHO 		=@echo
endif
# 测试二进制文件目录
TEST_BIN =	/usr/local/bin/luajit
# 测试文件目录
TEST_SHL =	./src/test.lua

# 指定CFLAGS
CFLAGS		?=-O3
# 指定编译选项
ADD_CFLAGS	+=$(CFLAGS) -fPIC

# addprefix 把前缀加到每个成员前面
OBJ = $(addprefix $(OBJ_DIR)/, lua_interface.o)

# 把pcm2wav改名
#all : $(OUT_LIB)
#	mv $(OUT_DIR)/luattsutil.so $(OUT_DIR)/pcm2wav.so 

# .obj 目录不存在就新建该目录,并在该目录中将libs目录 ? 中的.c 文件生成.o 文件
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@-if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_BLUE)"$(<:.c=.o)"$(COLOR_NONE)
	#g++ -std=c++11 $(ADD_CFLAGS) $(INC_DIR) -c $< -o $@
	gcc $(ADD_CFLAGS) $(INC_DIR) -c $< -o $@

# 把.obj中的.o 文件编译生成.so 文件
$(OUT_LIB) : $(OBJ)
	@-if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi
	#g++ -std=c++11 $(LIBFLAGS) $(ADD_CFLAGS)  $^ $(ADD_LIBA) $(LIB_DIR)  -o $@ $(ADD_LIBS)
	gcc $(LIBFLAGS) $(ADD_CFLAGS)  $^ $(ADD_LIBA) $(LIB_DIR)  -o $@ $(ADD_LIBS)
	$(ECHO) $(COLOR_GREEN)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

# 删除操作
clean :
	@rm -rf $(OBJ_DIR)
	@rm -rf $(OUT_LIB)
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

.PHONY: all
