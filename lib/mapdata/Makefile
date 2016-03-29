LIB_DIR=$(shell pwd)
OBJ = libmappmr.a \
      libmappoi.a \
      libmapsegment.a \
      libmaptopo.a \

all:$(OBJ)
	-@echo "Done LIB!"

libmappmr.a:
	cd libmappmr && $(MAKE)
	cd $(LIB_DIR)
	mv $(LIB_DIR)/libmappmr/libmappmr.a $(LIB_DIR)/../$@

libmappoi.a:
	cd libmappoi && $(MAKE)
	cd $(LIB_DIR)
	mv $(LIB_DIR)/libmappoi/libmappoi.a $(LIB_DIR)/../$@

libmapsegment.a:
	cd libmapsegment && $(MAKE)
	cd $(LIB_DIR)
	mv $(LIB_DIR)/libmapsegment/libmapsegment.a $(LIB_DIR)/../$@

libmaptopo.a:
	cd libmaptopo && $(MAKE)
	cd $(LIB_DIR)
	mv $(LIB_DIR)/libmaptopo/libmaptopo.a $(LIB_DIR)/../$@

clean:
	$(MAKE) -C $(LIB_DIR)/libmappmr distclean
	rm -f ../libmappmr.a
	$(MAKE) -C $(LIB_DIR)/libmappoi distclean
	rm -f ../libmappoi.a
	$(MAKE) -C $(LIB_DIR)/libmapsegment distclean
	rm -f ../libmapsegment.a
	$(MAKE) -C $(LIB_DIR)/libmaptopo distclean
	rm -f ../libmaptopo.a

