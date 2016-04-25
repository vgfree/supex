APPS    = traffic_draw_test
DRAWSO	= ../libtrafficd.so

UTILDIR  = ./c-hashtable/util
CFLAGS  = -Wall -g -O3
LDFLAGS = -lm -lhts
INCS     = -I$(UTILDIR)
LIBDIR  = -L$(UTILDIR) -L../
#CFLAGS  += `pkg-config gtk+-2.0 --cflags`
#LDFLAGS += `pkg-config gtk+-2.0 --libs`

CFLAGS  += `pkg-config cairo --cflags`
LDFLAGS += `pkg-config cairo --libs` -llua
SOCLAGS = -fPIC -shared

ADD_OBJ +=
BIN=./bin_$(APPS)
BASE_OBJ = $(addprefix $(BIN)/, \
           $(ADD_OBJ) \
	   traffic_draw.o \
	   loadgps.o \
	   sg_line.o \
	   sg_analyzer.o \
	   gps_mapping.o \
	   sg_info.o \
	   sg_node.o \
	   sg_text.o \
	   sg_adjust.o \
	   utils_draw.o \
           )

all: statics JOB 

JOB: $(BASE_OBJ)
	gcc $(CFLAGS) $(LIBDIR) $(SOCLAGS) $^ $(LDFLAGS) -o $(DRAWSO)

$(BIN)/%.o :./%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(INCS) $(CFLAGS) $(LIBDIR) $(LDFLAGS) $(SOCLAGS) -c $< -o $@

statics:
	cd $(UTILDIR);  $(MAKE) libhts.a

test_main: ./test/test_main.c
	gcc  -I./ $(CFLAGS) $(LIBDIR) $^ -Wl,-rpath=../ -lluamap -o $(APPS)
	#gcc  -I./ $(CFLAGS) $(LIBDIR) $^ $(LDFLAGS) -lluamap -o $(APPS)

clean:
	rm -rf $(BIN)
	rm -f $(APPS)
	rm -f $(DRAWSO)
	cd $(UTILDIR);  $(MAKE) clean
