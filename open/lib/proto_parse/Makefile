
all:libproto.a luaproto.so

libproto.a:proto_lua.c
	gcc -c proto.c 
	ar cr libproto.a proto.o
luaproto.so:proto_lua.c libproto.a
	gcc -fPIC -I/usr/local/include/luajit-2.0 -I./ -c proto_lua.c -o proto_lua.o
	gcc -fPIC -L/usr/local/lib -L./ -I./ -lluajit-5.1 -lm -shared -o luaproto.so proto_lua.o proto.c
clean:
	rm -f *.o *.a *.so

