OPT = -D_REENTRANT -D_GNU_SOURCE -DDEBUG -DBASE_THREADSAFE

all:
	gcc -g $(OPT) -lzmq 1.c -o 1.exe
	gcc -g $(OPT) -lzmq 2.c -o 2.exe
	gcc -g $(OPT) -lzmq 3.c -o 3.exe
	gcc -g $(OPT) -lczmq a.c -o a.exe
	gcc -g $(OPT) -lczmq b.c -o b.exe
	gcc -g $(OPT) -lczmq c.c -o c.exe
clean:
	rm -rf *.exe
