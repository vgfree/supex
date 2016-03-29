
CFLAGS = -g -shared -fPIC
ALL_INCS = -I /usr/local/include/luajit-2.0/ -I /usr/include/ -I ./src -I ./http-parser -I.
ALL_LIBS = -lluajit-5.1

SRC = http-parser/http_parser.c http-parser/http_parser.h \
      src/hm_parser.c src/hm_parser.h \
      src/hm_buffer.c src/hm_buffer.h \
      src/hm_array.c src/hm_array.h

SRC += hm_header_ids.h src/http_message.nobj.c

http_message.so:$(SRC)
	gcc $(CFLAGS) $^ -o $@ $(ALL_INCS) $(ALL_LIBS)


clean:
	rm -rf http_message.so 	hm_header_ids.h src/http_message.nobj.c src/http_message.nobj.ffi.lua



hm_header_ids.h:
	gperf  -L C hm_header_ids.gperf > $@

src/http_message.nobj.c:
	lua LuaNativeObjects/native_objects.lua -outpath ./src -gen  lua  http_message.nobj.lua

