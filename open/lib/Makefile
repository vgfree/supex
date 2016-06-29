LIB_DIR=$(shell pwd)
#socket.so		when use system install, please be careful the socket lib has no problem.
OBJ = cutils.so \
      socket.so \
      lfs.so \
      zlib.so \
      bit32.so \
      cjson.so \
      conhash.so \
      luasql.so \
      conv.so \
      libhttps.so \
      libluadecoder.so \
      lualog.so \
      luakv.so \
      mfptppack.so \
      zmq.so \
      http_message.so \
      zklua.so	

export ADDITION_LPATH = -L$(LIB_DIR)/../../lib/

#export ADDITION_CFLAGS = -DUSE_MEMHOOK -export-dynamic		\
		-Wl,-wrap,malloc -Wl,-wrap,calloc \
		-Wl,-wrap,realloc -Wl,-wrap,free \
		-Wl,-wrap,strdup

export ADDITION_LIBA = -lmemhook -lmini -ldl

all:$(OBJ)
	-@echo "Done LIB!"

cutils.so:
	$(MAKE) -C $(LIB_DIR)/cutils
	mv $(LIB_DIR)/cutils/$@ $(LIB_DIR)

socket.so:
	$(MAKE) -C $(LIB_DIR)/luasocket
	mv $(LIB_DIR)/luasocket/src/socket.so.3.0-rc1  $(LIB_DIR)/$@

lfs.so:
	$(MAKE) -C $(LIB_DIR)/luafilesystem
	mv $(LIB_DIR)/luafilesystem/src/$@  $(LIB_DIR)

zlib.so:
	$(MAKE) -C $(LIB_DIR)/lua-zlib linux
	mv $(LIB_DIR)/lua-zlib/$@ $(LIB_DIR)

bit32.so:
	$(MAKE) -C $(LIB_DIR)/Lua-bit32
	mv $(LIB_DIR)/Lua-bit32/$@ $(LIB_DIR)

cjson.so:
	$(MAKE) -C $(LIB_DIR)/lua-cjson-2.1.0
	mv $(LIB_DIR)/lua-cjson-2.1.0/$@ $(LIB_DIR)

conhash.so:
	$(MAKE) -C $(LIB_DIR)/libconhash
	mv $(LIB_DIR)/libconhash/bin/$@ $(LIB_DIR)

luasql.so:
	$(MAKE) -C $(LIB_DIR)/luasql-master
	mv $(LIB_DIR)/luasql-master/src/$@ $(LIB_DIR)

conv.so:
	$(MAKE) -C $(LIB_DIR)/conv
	mv $(LIB_DIR)/conv/$@ $(LIB_DIR)

libtcp.so:
	$(MAKE) -C $(LIB_DIR)/dkldb
	mv $(LIB_DIR)/dkldb/$@ $(LIB_DIR)

libluadecoder.so:
	$(MAKE) -C $(LIB_DIR)/decoder
	mv $(LIB_DIR)/decoder/$@ $(LIB_DIR)

zmq.so:
	$(MAKE) -C $(LIB_DIR)/lua-zmq
	mv $(LIB_DIR)/lua-zmq/$@ $(LIB_DIR)

http_message.so:
	$(MAKE) -C $(LIB_DIR)/lua-http-message
	mv $(LIB_DIR)/lua-http-message/$@ $(LIB_DIR)

lualog.so:
	$(MAKE) -C $(LIB_DIR)/lua-log CFLAGS="-g -O2 -D_GNU_SOURCE -DADD_LENGTH_FIELD"
	mv $(LIB_DIR)/lua-log/$@ $(LIB_DIR)

luakv.so:
	$(MAKE) -C $(LIB_DIR)/luakv CFLAGS="-g -O2"
	mv $(LIB_DIR)/luakv/lib/$@ $(LIB_DIR)
mfptppack.so:
	$(MAKE) -C $(LIB_DIR)/lua-mfptp
	mv $(LIB_DIR)/lua-mfptp/$@ $(LIB_DIR)

luaflif.so:
	$(MAKE) -C $(LIB_DIR)/lua-FLIF	
	cp $(LIB_DIR)/lua-FLIF/lua_api/lib/$@ $(LIB_DIR)	

libhttps.so:
	tar -xzvf polarssl-1.2.8.tar.gz && mkdir build && cd build && ccmake ../polarssl-1.2.8/ && make && cp programs/ssl/libhttps.so ../

zklua.so:
	$(MAKE) -C $(LIB_DIR)/zklua
	cp $(LIB_DIR)/zklua/$@ $(LIB_DIR)
clean:
	rm -rf ./*.so
	$(MAKE) -C $(LIB_DIR)/lua-cjson-2.1.0 clean
	$(MAKE) -C $(LIB_DIR)/luasql-master clean
	$(MAKE) -C $(LIB_DIR)/libconhash clean
	$(MAKE) -C $(LIB_DIR)/luasocket clean
	$(MAKE) -C $(LIB_DIR)/luafilesystem clean
	$(MAKE) -C $(LIB_DIR)/decoder clean
	$(MAKE) -C $(LIB_DIR)/lua-log clean
	$(MAKE) -C $(LIB_DIR)/luakv clean
	$(MAKE) -C $(LIB_DIR)/lua-mfptp clean
	$(MAKE) -C $(LIB_DIR)/lua-zmq clean
	$(MAKE) -C $(LIB_DIR)/lua-http-message clean
	cd ./lua-zlib && make clean
	cd ./Lua-bit32 && make clean
	cd ./lua-FLIF && make clean
	cd ./zklua && make clean
	-if [ -d ./build ];then rm -rf ./build; fi
	-if [ -d ./polarssl-1.2.8 ];then rm -rf ./polarssl-1.2.8; fi

