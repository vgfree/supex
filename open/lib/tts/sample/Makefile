all:sample sample_line sample_total sample_multi_thread

sample:sample.cpp
	g++ $^ ../libs/libusc.so -Wall -O2 -o $@ -L. -lrt

sample_line:sample_line.cpp
	g++ $^ ../libs/libusc.so -Wall -O2 -o $@ -L. -lrt

sample_total:sample_total.cpp
	g++ $^ ../libs/libusc.so -Wall -O2 -o $@ -L. -lrt

sample_multi_thread:sample_multi_thread.cpp
	g++ $^ ../libs/libusc.so -Wall -O2 -o $@ -L. -lrt -lpthread

clean:
	rm -f sample sample_line sample_total sample_multi_thread
	rm -f sample *.o *.*~ -f
