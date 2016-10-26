ffmpeg安装

依赖:
	yum install automake autoconf make gcc gcc-c++ libtool zlib zlib-devel curl curl-devel alsa-lib alsa-lib-devel gettext gettext-devel expat expat-devel lame-devel opencore-amr-devel

#########################################################################################
#yasm(汇编编译器)可选择安装:									#
#	wegt  http://www.tortall.net/projects/yasm/releases/yasm-0.6.0.tar.gz		#
#	tar xzvf yasm-0.6.0.tar.gz							#
#	cd yasm-0.6.0									#
#	./configure --prefix=/usr/local/yasm						#
#	make										#
#	make instal									#
#########################################################################################


http://www.cnblogs.com/xiaofengfeng/p/3573025.html

CC
1、  下载ffmpeg源代码：http://ffmpeg.org/download.html
2、  编译安装ffmpeg，注意configure命令后面所要添加的参数，可以通过执行configure --help命令查看后面所允许带的参数

tar -xjvf ffmpeg-2.8.4.tar.bz2
cd ffempg
./configure     --prefix=/usr --bindir=/usr/bin --datadir=/usr/share/ffmpeg --incdir=/usr/include/ffmpeg --libdir=/usr/lib64 --mandir=/usr/share/man --arch=x86_64 --optflags='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic' --enable-bzlib --disable-crystalhd --disable-indev=jack --enable-libfreetype --enable-libmp3lame  --enable-x11grab --enable-avfilter --enable-avresample --enable-postproc --enable-pthreads --enable-static --enable-gpl --disable-debug --disable-stripping --shlibdir=/usr/lib64 --enable-runtime-cpudetect --enable-version3 --disable-doc --enable-libopencore-amrnb --enable-libopencore-amrwb --enable-libmp3lame --disable-ffserver --disable-ffplay --disable-yasm --disable-ffprobe #--enable-libpulse --enable-libvpx


make
make install



wav--->amr
./ffmpeg -v fatal -i 1c4d32b4-a884-11e5-9d5e-000c29c75af0.wav -y  -ab 5.15k -ar 8000 -ac 1 1c4d32b4-a884-11e5-9d5e-000c29c75af0.amr


mp3--->amr
./ffmpeg -v fatal -i 1c4d32b4-a884-11e5-9d5e-000c29c75af0.mp3 -y  -ab 5.15k -ar 8000 -ac 1 1c4d32b4-a884-11e5-9d5e-000c29c75af0.amr


