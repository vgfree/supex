LIB_DIR=$(shell pwd)
OBJ = libmini.a \
      libmemhook.a \
      libev.a \
      libjson-c.a \
      libcjson.a \
      libtopo.a \
      libsnappy.a \
      libleveldb.a \
      libucmq.a \
      liblrucache.a \
      libkv.a \
      libhiredis.a \
      libtopoi.a \
      libzmq.a \
      libxmq.a \
      libnetmod.a \
      libzookeeper_mt.a \
      libsqlite3.a

all:$(OBJ)
	-@echo "Done LIB!"

libxmq.a:
	$(MAKE) -C $(LIB_DIR)/libxmq
	mv $(LIB_DIR)/libxmq/$@ $(LIB_DIR)
libnetmod.a:
	$(MAKE) -C $(LIB_DIR)/libnetmod
	mv $(LIB_DIR)/libnetmod/$@ $(LIB_DIR)

libzmq.a:
	cd ./zeromq-4.1.4 && ./configure CC=g++ --with-pic --enable-shared=no --without-libsodium
	$(MAKE) -j16 -C $(LIB_DIR)/zeromq-4.1.4
	mv $(LIB_DIR)/zeromq-4.1.4/.libs/$@ $(LIB_DIR)

#-DCORO_UCONTEXT"
#if you want use libevcoro to operate a file-descriptor of libzmq, then you need to define a macro like "FEED_EVENT"
libevcoro.a:
	$(MAKE) -C $(LIB_DIR)/libevcoro CFLAGS="-g -O1 -DFEED_EVENT"
	mv $(LIB_DIR)/libevcoro/lib/$@ $(LIB_DIR)

#-DCORO_UCONTEXT"
libcoro.a:
	cd ./libcoro && mkdir -p ./tmp/ && cd ./tmp/ && cmake .. -DCMAKE_C_FLAGS="-g -O1"
	$(MAKE) -C $(LIB_DIR)/libcoro/tmp
	mv $(LIB_DIR)/libcoro/tmp/config.h $(LIB_DIR)/libcoro
	mv $(LIB_DIR)/libcoro/tmp/$@ $(LIB_DIR)

libev.a:
	cd ./libev && sh ./autogen.sh && ./configure --enable-shared=no
	$(MAKE) -C $(LIB_DIR)/libev
	mv $(LIB_DIR)/libev/.libs/$@ $(LIB_DIR)

libscco.a:
	$(MAKE) -C $(LIB_DIR)/libscco
	mv $(LIB_DIR)/libscco/$@ $(LIB_DIR)

libjson-c.a:
	cd ./json-c && ./autogen.sh && ./configure --enable-shared=no
	$(MAKE) -j8 -C $(LIB_DIR)/json-c
	mv $(LIB_DIR)/json-c/.libs/$@ $(LIB_DIR)

libcjson.a:
	$(MAKE) -C $(LIB_DIR)/cJSON
	mv $(LIB_DIR)/cJSON/$@ $(LIB_DIR)

libtopo.a:
	$(MAKE) -C $(LIB_DIR)/libtopo
	mv $(LIB_DIR)/libtopo/$@ $(LIB_DIR)
libtopoi.a:
	$(MAKE) -C $(LIB_DIR)/libtopoi
	mv $(LIB_DIR)/libtopoi/$@ $(LIB_DIR)

libsnappy.a:
	-if [ -d ./snappy-1.1.1 ];then  echo "snappy-1.1.1 exist!"; else  tar -xzvf ../third-party/snappy-1.1.1.tar.gz; fi
	cd ./snappy-1.1.1 && ./configure
	$(MAKE) -j8 -C $(LIB_DIR)/snappy-1.1.1
	mv $(LIB_DIR)/snappy-1.1.1/.libs/$@ $(LIB_DIR)

libleveldb.a:
	$(MAKE) -j8 -C $(LIB_DIR)/leveldb-1.19
	mv $(LIB_DIR)/leveldb-1.19/out-static/$@ $(LIB_DIR)

libucmq.a:
	$(MAKE) -C $(LIB_DIR)/libucmq
	mv $(LIB_DIR)/libucmq/$@ $(LIB_DIR)

liblrucache.a:
	$(MAKE) -C $(LIB_DIR)/liblrucache
	mv $(LIB_DIR)/liblrucache/$@ $(LIB_DIR)

libkv.a:
	$(MAKE) -C $(LIB_DIR)/libkv release
	mv $(LIB_DIR)/libkv/release/$@ $(LIB_DIR)

libhiredis.a:
	$(MAKE) -C $(LIB_DIR)/hiredis
	mv $(LIB_DIR)/hiredis/$@ $(LIB_DIR)

libmemhook.a:
	$(MAKE) -C $(LIB_DIR)/libmemhook CFLAGS="-g -O2"
	mv $(LIB_DIR)/libmemhook/lib/$@ $(LIB_DIR)

libmini.a:
	$(MAKE) -j8 -C $(LIB_DIR)/libmini CFLAGS="-g -DSLOG_ADD_LENGTH_FIELD -DSQUEUE_USE_RWLOCK"
	mv $(LIB_DIR)/libmini/lib/$@ $(LIB_DIR)

libzookeeper_mt.a:
	-if [ -d ./zookeeper-3.4.6 ];then echo "zookeeper-3.4.6 exist!"; else tar -zxvf ../third-party/zookeeper-3.4.6.tar.gz; fi
	cd $(LIB_DIR)/zookeeper-3.4.6/src/c && ./configure
	$(MAKE) -j8 -C $(LIB_DIR)/zookeeper-3.4.6/src/c
	mv $(LIB_DIR)/zookeeper-3.4.6/src/c/.libs/$@ $(LIB_DIR)

libsqlite3.a:
	cd ./sqlite-autoconf-3090200 && autoreconf --force --install && ./configure
	$(MAKE) -C $(LIB_DIR)/sqlite-autoconf-3090200
	mv $(LIB_DIR)/sqlite-autoconf-3090200/.libs/$@ $(LIB_DIR)


test:
	gcc -g -o example -I./libev -I./hiredis -L./  hiredis/examples/example-libev.c  -lev -lhiredis -lm

clean:
	-if [ -f ./libev/Makefile ];then cd ./libev && make distclean; else cd ./libev && sh ./autogen.sh && ./configure && make distclean; fi
	-if [ -f ./zeromq-4.1.4/Makefile ];then cd ./zeromq-4.1.4 && make distclean; else cd ./zeromq-4.1.4 && sh ./autogen.sh && ./configure CC=g++ -enable-shared=no && make distclean; fi
	-if [ -f json-c/Makefile ];then cd ./json-c && make distclean; fi
	-if [ -d ./snappy-1.1.1 ];then rm -rf ./snappy-1.1.1; fi
	-if [ -f sqlite-autoconf-3090200/Makefile ];then cd ./sqlite-autoconf-3090200 && make distclean; fi
	$(MAKE) -C $(LIB_DIR)/leveldb-1.19 clean
	$(MAKE) -C $(LIB_DIR)/cJSON clean
	$(MAKE) -C $(LIB_DIR)/libucmq clean
	$(MAKE) -C $(LIB_DIR)/libtopo clean
	$(MAKE) -C $(LIB_DIR)/libtopoi clean
	$(MAKE) -C $(LIB_DIR)/liblrucache clean
	$(MAKE) -C $(LIB_DIR)/hiredis clean
	$(MAKE) -C $(LIB_DIR)/libkv clean
	$(MAKE) -C $(LIB_DIR)/libmemhook distclean
	$(MAKE) -C $(LIB_DIR)/libmini distclean
	$(MAKE) -C $(LIB_DIR)/libxmq clean
	$(MAKE) -C $(LIB_DIR)/libnetmod clean
	-if [ -d ./zookeeper-3.4.6 ];then rm -rf ./zookeeper-3.4.6; fi
	rm -rf $(OBJ)

