build:
	gcc -g -o Xldb-reader ldb.c -L./leveldb-1.14.0 -lleveldb -pthread -I ./leveldb-1.14.0/include \
		-DTEST_RANDOM_READ=0 \
		-DTEST_ITER_READ=1 \
		-DLONG_TIME_READ=1 \
		-DRANGE_GET_KEY_CNT=144

vg:
	valgrind --tool=memcheck --leak-check=yes -v ./Xldb* > out

gen:
	sh gen.sh

push:
	git push origin HEAD:refs/for/master

r: ldb.c Makefile
	gcc -g -o Xldb-reader ldb.c -L./leveldb-1.14.0 -lleveldb -pthread -I ./leveldb-1.14.0/include \
		-DTEST_RANDOM_READ=0 \
		-DTEST_ITER_READ=1 \
		-DLONG_TIME_READ=1 \
		-DRANGE_GET_KEY_CNT=144
	nohup ./Xldb-reader &

w: ldb.c Makefile
	gcc -g -o Xldb-writer ldb.c -L./leveldb-1.14.0 -lleveldb -pthread -I ./leveldb-1.14.0/include \
		-DSYNC_PUT=1 \
		-DBATCH_PUT=1 \
		-DDIRECT_PUT=0 \
		-DCOMPRESSION=1 \
		-DBATCH_WRITE_MB_BYTES=1024
	rm db -rf
	./Xldb-writer

clean_data:
	rm *.ldb -f

clean:
	rm X*
