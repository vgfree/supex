INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev \
	-I../../lib/json-c -I../../lib/http-parser \
	-I../../lib/libscco -I../../engine \
	-I../../engine/major -I../../engine/minor -I./src \
	-I../../lib/libmemhook/src/ -I../../lib/libmini/include \
	-I../../lib/libmini/src \
	-I../../lib/cJSON

LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++ -ldl -lcrypto 
LIBA=-ljson-c -lsupex_base  -lmini -lev -lkv -lscco -lcjson 

#<------------------------>#
#<------------------------>#

sample:
	gcc -g  async_http_fifo.c  $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_http_fifo
	gcc -g  async_http_coro.c  $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_http_coro
	gcc -g  async_redis_fifo.c $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_redis_fifo
	gcc -g  async_redis_coro.c $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_redis_coro
	gcc -g  async_redis_http.c $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_redis_http
	gcc -g  async_peak.c 	   $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o async_peak

test:
	gcc -g  libev_client.c 	   $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o libev_client
	gcc -g  randmap_test.c 	   $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o randmap_test 

clean:
	rm -rf libev_client randmap_test async_http_fifo async_http_coro async_redis_fifo async_redis_coro async_redis_http async_peak
