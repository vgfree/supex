#!/bin/bash

#1) install libboost
cd graphlabapi/
yum -y install boost boost-devel

#2) install itpp -- this step is optional
yum install lapack autoconf autogen libtool
ln -s /usr/lib64/liblapack.so.3.2.1 /usr/lib64/liblapack.so
ln -s /usr/lib64/libblas.so.3.2.1 /usr/lib64/libblas.so

yum groupinstall "Development Tools"

wget http://sourceforge.net/projects/itpp/files/itpp/4.2.0/itpp-4.2.tar.gz
tar xvzf itpp-4.2.tar.gz
cd itpp-4.2/
./autogen.sh
./configure --without-fft --with-blas=/usr/lib64/libblas.so.3 --with-lapack=/usr/lib64/liblapack.so.3 CFLAGS=-fPIC CXXFLAGS=-fPIC CPPFLAGS=-fPIC

make && make install

#3) install Mercurial
yum -y install mercurial

#4) install cmake
#yum -y install cmake

#5) configure using It++
cd ../
./configure --bootstrap --itpp_include_dir=/usr/local/include/ --itpp_dynamic_link_dir=/usr/local/lib/libitpp.so --itpp_lapack_dir=/usr/lib64/

#Alternatively -- you can configure and compile using Eigen:
#./configure --bootstrap --eigen

#6) compile
cd release/
make -j4

#7) test GraphLab
cd ../tests/
export LD_LIBRARY_PATH=/usr/local/lib/
cd release/tests/
./runtests.sh

echo "Completed !"

