COLOR_NONE            = "\x1B[m"
COLOR_RED             = "\x1B[0;31m"
COLOR_GREEN           = "\x1B[0;32m"
COLOR_BLUE            = "\x1B[0;34m"

HOST =		$(shell uname | tr '[A-Z]' '[a-z]')

LDFLAGS =	$(ADDITION_CFLAGS)
LIBS =		$(ADDITION_LIBA)
LPATH =		$(ADDITION_LPATH)

TARGET =	lualog.so
OBJS = 		lualog.o lualog_utils.o

INCLUDEDIR =	-I/usr/local/include/luajit-2.0
LPATH +=	-L/usr/local/lib
LIBS +=		-lluajit-5.1 -lm -lpthread
TEST_BIN =	/usr/local/bin/luajit
TEST_SHL =	./test.lua

ifeq ($(HOST), darwin)
ECHO = @echo
SHARE = -bundle -undefined dynamic_lookup
else
ECHO = @echo -e
SHARE = -shared
endif

CFLAGS ?= -O2

all : $(TARGET) lualog-test


$(TARGET) : $(OBJS)
	@gcc -O2 -fpic $(LDFLAGS) $(LPATH) $(SHARE) $^ -o $@ $(LIBS)
	$(ECHO) $(COLOR_GREEN)"\n\tBUILD\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

%.o : %.c
	$(ECHO) $(COLOR_GREEN)"\t\t- COMPILE\t===>\t"$(COLOR_BLUE)"$(<:.c=.o)"$(COLOR_NONE)
	@gcc $(CFLAGS) -fpic $(INCLUDEDIR) -c $< -o $@

lualog-test : $(TARGET)
	$(TEST_BIN) $(TEST_SHL)
	$(ECHO) $(COLOR_GREEN)"\n\tTEST \t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)

clean:
	@-rm -rf ./log
	@-rm -rf *.log
	@-rm -rf *.o
	@-rm -rf *.so
	$(ECHO) $(COLOR_GREEN)"\n\tCLEAN\t >>> "$(COLOR_RED)"$@"$(COLOR_GREEN)" <<< COMPLETE"$(COLOR_NONE)
.PHONY: all
