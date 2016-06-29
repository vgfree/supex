LIB_DIR=$(shell pwd)
OBJ = libzookeeper_mt.a

all:$(OBJ)
	-@echo "Done LIB!"

libzookeeper_mt.a:
	mkdir -p build
	tar zxvf zookeeper-3.4.6.tar.gz
	cd ./zookeeper-3.4.6/src/c && ./configure --prefix=$(LIB_DIR)/build
	$(MAKE) -C $(LIB_DIR)/zookeeper-3.4.6/src/c
	$(MAKE) -C $(LIB_DIR)/zookeeper-3.4.6/src/c install
	mv $(LIB_DIR)/zookeeper-3.4.6/src/c/.libs/$@ $(LIB_DIR)

clean:
	rm -rf $(OBJ)
	-if [ -d zookeeper-3.4.6 ];then rm -rf ./zookeeper-3.4.6; fi
	-if [ -d build ];then rm -rf ./build; fi

