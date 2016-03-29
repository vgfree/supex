# Copyright (c) 2010 Aleksey Yeschenko <aleksey@yeschenko.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
CC = gcc -g
CFLAGS = `pkg-config luajit --cflags` -fPIC -O3 -Wall -I/usr/local/include/luajit-2.0
LFLAGS = -shared -fPIC #`pkg-config --libs --cflags libzmq`
INSTALL_PATH = `pkg-config luajit --variable=INSTALL_CMOD`

CFLAGS	  +=-I./zeromq-4.1.4/include/

all: zmq.so

libzmq.a:
	tar -xzvf zeromq-4.1.4.tar.gz
	cd ./zeromq-4.1.4 && ./configure CC=g++ --with-pic --enable-shared=no --without-libsodium
	$(MAKE) -j16 -C zeromq-4.1.4
	mv zeromq-4.1.4/.libs/$@ .


zmq.lo: zmq.c
	$(CC) -o zmq.lo -c $(CFLAGS) zmq.c


zmq.so: libzmq.a zmq.lo
	ar -x libzmq.a
	$(CC) $(LFLAGS) zmq.lo *.o -o zmq.so -lm -lpthread -lrt -lstdc++ -ldl
	rm *.o


install: zmq.so
	install -D -s zmq.so $(INSTALL_PATH)/zmq.so


clean:
	rm -f libzmq.a zmq.so zmq.lo
	-if [ -d ./zeromq-4.1.4 ];then rm -rf ./zeromq-4.1.4; fi
