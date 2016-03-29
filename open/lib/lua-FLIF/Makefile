CC = gcc -g
CFLAGS = `pkg-config luajit --cflags` -fPIC -O3 -Wall -I/usr/local/include/luajit-2.0
LFLAGS = -shared -fPIC
INSTALL_PATH = `pkg-config luajit --variable=INSTALL_CMOD`

CFLAGS	  +=-I./FLIF-master/include/

all: libflif.so libflif_interface.so luaflif.so

libflif.so:
	unzip FLIF-master.zip
	cd ./FLIF-master
	$(MAKE) -j16 -C FLIF-master

libflif_interface.so: 
	gcc -fPIC -shared -o $@ ./convert.c -L./FLIF-master/src -lflif -I ./FLIF-master/src/library/ -w
	#g++ -std=c++11 -fPIC -shared -o $@ ./convert.cpp -L./FLIF-master/src -lflif -I ./FLIF-master/src/library/ -w


luaflif.so:
	cp convert.h ./lua_api/src
	$(MAKE) -C ./lua_api	

install:
	cd ./FLIF-master && make install

clean:
	rm -rf FLIF-master
	rm -rf *.so
	cd lua_api && make clean


