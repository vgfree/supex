
#!/bin/sh

LIB_PATH=/usr/local/lib
INC_PATH=/usr/include/
SRC_PATH=../src/

case "$1" in
    netmod)
	# Down compile with dynamic library.
	gcc -g test_server.c ../src/netmod.c ../src/cutil.c ../src/cqueue.c -o server -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	gcc -g test_client.c ../src/netmod.c ../src/cutil.c ../src/cqueue.c -o client -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	
	#gcc -g test_server.c -o server -I$INC_PATH -I$SRC_PATH -L../src/target/ -lnetmod -L$LIB_PATH 
	#gcc -g test_client.c -o client -I$INC_PATH -I$SRC_PATH -L../src/target/ -lnetmod -L$LIB_PATH 

	# Down compile with static library.
	#gcc -g test_server.c ../src/netmod.c ../src/cutil.c ../src/libzmq.a -o server -I$INC_PATH -I$SRC_PATH -L$LIB_PATH -lstdc++ -lrt -lsodium
	#gcc -g test_client.c ../src/netmod.c ../src/cutil.c ../src/libzmq.a -o client -I$INC_PATH -I$SRC_PATH -L$LIB_PATH -lstdc++ -lrt -lsodium
	;;
    netmod1)
	gcc -g test_server1.c ../src/cutil.c -o server1 -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	gcc -g test_client1.c ../src/cutil.c -o client1 -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	;;
    netmod2)
	gcc -g test_server2.c ../src/cutil.c -o server2 -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	gcc -g test_client2.c ../src/cutil.c -o client2 -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	;;
    netmod3)
	gcc -g test_server3.c ../src/netmod.c ../src/cutil.c ../src/cqueue.c ../src/clog.c -o server3 -I$INC_PATH -I$SRC_PATH -lzmq -L$LIB_PATH 
	;;
    router)
	gcc -g test_router.c ../src/cutil.c -o router -I../src -lzmq
	;;
    proxy)
	gcc -g test_proxy.c ../src/cutil.c -o proxy -I../src -lzmq
	;;
    *)
	echo "Usage: ./compiler.sh [netmod[1|2|3]|router|proxy|...]"
	exit
	;;
esac
