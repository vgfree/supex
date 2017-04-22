#!/bin/bash

sudo yum install ncurses-devel -y
sudo yum install bison -y
mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/home/baoxue/bin/mysql -DMYSQL_DATADIR=/home/baoxue/var/mysql -DWITH_ARCHIVE_STORAGE_ENGINE=on -DWITH_BLACKHOLE_STORAGE_ENGINE=on -DWITH_READLINE=on -DWITH_SSL=system -DWITH_ZLIB=system -DWITH_XTRADB_STORAGE_ENGINE=on
make
make install
