
test:
#	gcc -fdiagnostics-color=auto -g \
		test_evcoro_async_tasks.c \
		-lpthread  -levcs -levcoro -lcoro -lev -lmini -lm -ldl -lluajit-5.1 \
		-I. -I../../lib/libevcs/include/engine/ -I../../lib/libev -I../../lib/libevcoro/include -I../../lib/libcoro/include/ -I../../lib/libmini/src -I/usr/local/include/luajit-2.0 \
		-L../../lib -L/usr/local/lib
#	gcc -fdiagnostics-color=auto -g \
		test_thread_pool_loop.c \
		-lpthread  -levcs -levcoro -lcoro -lev -lmini -lm -ldl -lluajit-5.1 \
		-I. -I../../lib/libevcs/include/engine/ -I../../lib/libev -I../../lib/libevcoro/include -I../../lib/libcoro/include/ -I../../lib/libmini/src -I/usr/local/include/luajit-2.0 \
		-L../../lib -L/usr/local/lib	

	gcc -fdiagnostics-color=auto -g \
		link_cfg_parser.c \
		custom_hash.c \
		link_hashmap.c \
		link_xpool.c \
		test_link.c \
		-lpthread  -levcs -levcoro -lcoro -lev -lmini -lm -ldl -lluajit-5.1 -ljson-c -lconhash\
		-I. -I../../lib/libevcs/include/engine/ -I../../lib/libev -I../../lib/libevcoro/include -I../../lib/libcoro/include/ -I../../lib/libmini/src -I/usr/local/include/luajit-2.0 -I../../lib/json-c -I./libconhash\
		-L../../lib -L/usr/local/lib -L./libconhash/bin
