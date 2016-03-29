INC_DIR = -I/data/supex/lib/cJSON -I/data/supex/lib/libmini/include/ -I/data/supex/lib/hiredis/ -I/data/supex/lib/libkv/release
LIB_DIR = -L/data/supex/lib -L/data/supex/lib/cJSON -L/data/supex/lib/hiredis/ -L./
LIB_OBJECT=libcjtopbinterface.a
OBJECT=mirrtalk-transfer-data.pb.o cjson_topb.o
LIBS = -lrt -lm -lcjson -fPIC
Llibs = -lhiredis -lkv -lrt -lm -lcjson -lz
CFLAGS = -Wall -g -O0 \
	 -DOPEN_DEBUG

#PROTO = henanwl-transfer-data
PROTO = mirrtalk-transfer-data

SRC_DIR = .

SRC = $(addprefix $(SRC_DIR)/, \
      henanwl-transfer-data.pb.cc \
      cjson_topb.cc)

protoc_middleman: $(PROTO).proto
	protoc --cpp_out=. $<

pb: cjson_topb.cc
	pkg-config --cflags protobuf
	c++ $(CFLAGS) $(SRC) $(INC_DIR) $(LIB_DIR) $(LIBS) -lpthread -o pb `pkg-config --cflags --libs protobuf`

$(LIB_OBJECT):$(OBJECT)
	ar rs $@ $^ 
	cp $@ ../../../lib/

$(OBJECT):$(SRC)
	g++ -c $<  $(CFLAGS) $(SRC) $(INC_DIR) $(LIB_DIR) $(Llibs) `pkg-config --cflags --libs protobuf`

staticlib : $(LIB_OBJECT)

test: test.c 
	gcc $^ -o $@ $(CFLAGS) $(LIB_DIR) $(INC_DIR) -lcjtopbinterface $(Llibs) `pkg-config --cflags --libs protobuf`

#kvtest: loadredis.c redis_tomem.c
#Generationfile: redisfile.c redis_interface.c
filetokv: loadfile_tomem.c redis_kv.c
	gcc $^ -o $@ $(CFLAGS) $(LIB_DIR) $(INC_DIR) $(Llibs)

mysql: mysql_ops.c filter.c
	gcc $^ -o $@  $(CFLAGS) -L/usr/lib64/mysql $(LIB_DIR) $(INC_DIR)  -I/usr/include/mysql -lmysqlclient -lkv $(Llibs) $(LIBS)

count: count.c
	gcc $^ -o $@ $(CFLAGS) -L/usr/lib64/mysql $(LIB_DIR) $(INC_DIR)  -I/usr/include/mysql -lmysqlclient -lkv $(Llibs) $(LIBS)


clean:
	rm -rf $(PROTO).pb.cc
	rm -rf $(PROTO).pb.h
	rm -f test
	rm -f $(LIB_OBJECT)
	rm -f $(OBJECT)
	rm -f ./pb
	rm -f ../../../lib/$(LIB_OBJECT)
	rm -rf filetokv
	rm -f ./*.o
	rm -f ./mysql

