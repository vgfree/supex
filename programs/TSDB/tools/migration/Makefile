all:
	gcc -I../../lib/hiredis/ migration.c -g -L../../lib -lhiredis -O3 -o migration

clean:
	rm -rf migration
