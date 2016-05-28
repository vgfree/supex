NONE            = "\x1B[m"
GRAY            = "\x1B[0;30m"
LIGHT_GRAY      = "\x1B[1;30m"
RED             = "\x1B[0;31m"
LIGHT_RED       = "\x1B[1;31m"
GREEN           = "\x1B[0;32m"
LIGHT_GREEN     = "\x1B[1;32m"
YELLOW          = "\x1B[0;33m"
LIGHT_YELLOW    = "\x1B[1;33m"
BLUE            = "\x1B[0;34m"
LIGHT_BLUE      = "\x1B[1;34m"
PURPLE          = "\x1B[0;35m"
LIGHT_PURPLE    = "\x1B[1;35m"
CYAN            = "\x1B[0;36m"
LIGHT_CYAN      = "\x1B[1;36m"
WHITE           = "\x1B[0;37m"
LIGHT_WHITE     = "\x1B[1;37m"
######################################
str_kernel_version = $(shell uname -r)
get_kernel_version = $(shell echo $1 | sed 's/\./ /g' | sed 's/-/ /g')
fix_kernel_version = $(call get_kernel_version, $(str_kernel_version))

v_major = $(word 1, $(fix_kernel_version))
v_minor = $(word 2, $(fix_kernel_version))
v_emend = $(word 3, $(fix_kernel_version))

OPTIMIZE = 0

compare = $(shell if [ $1 -eq $2 ]; then echo ?; elif [ $1 -gt $2 ]; then echo 1; else echo 0; fi)
OPTIMIZE = $(call compare, $(v_major), 2)

ifeq ($(OPTIMIZE), ?)
OPTIMIZE = $(call compare, $(v_minor), 6)
ifeq ($(OPTIMIZE), ?)
OPTIMIZE = $(call compare, $(v_emend), 35)
endif
endif
ifeq ($(OPTIMIZE), ?)
OPTIMIZE = 1
endif

ifeq ($(OPTIMIZE), 1)
EXCESS_CFLAGS += -DOPEN_OPTIMIZE
endif
######################################

export OBJECT_SCENE ?= ONLINE
export SCCO_STACK_TYPE ?= -DSCCO_USE_STATIC_STACK

export HOME_PATH = $(shell pwd)
EXPORT_CFLAGS = -g
ifeq ($(OBJECT_SCENE), ONLINE)
EXPORT_CFLAGS += -O1
else
EXPORT_CFLAGS += -finstrument-functions
endif
EXPORT_CFLAGS += $(EXCESS_CFLAGS)
EXPORT_CFLAGS += -Wall \
	-DUSE_PIPE \
	-DUSE_SPINLOCK \
	-DCRZPT_OPEN_MSMQ \
	-DCRZPT_MSMQ_SELECT_ASYNC \
	-D_GNU_SOURCE \
	-DOPEN_POLLING \
	-DDEBUG \
	#-DSECTION_SEND \
	#-DOPEN_HASH \
	#-DOPEN_EQUAL \
	#-D_AO_CPU_NUMS=8 \
	#-DCRZPT_MSMQ_SELECT_SYNC \
	#-DUSE_MUTEX \
	#-DOPEN_TIME_OUT \
	#-static-libgcc -static-libstdc++

#<--内存管理钩子链接选项，如果不需要监控内存分配请去掉-->#
#EXPORT_CFLAGS += -DUSE_MEMHOOK \
	-export-dynamic \
	-Wl,-wrap,malloc -Wl,-wrap,calloc \
	-Wl,-wrap,realloc -Wl,-wrap,free \
	-Wl,-wrap,strdup

EXPORT_LIBA += -lmemhook -lmini -ldl
# -lgcov

export EXPORT_CFLAGS
export EXPORT_LIBA

openIMX := coreExchangeNode messageGateway useroptapi
openHLS := rmsapi hlsapi hlsldb
SRV := $(openIMX) $(openHLS) \
	drisamp drimode goby rtmiles rta \
	drisampapi drimodeapi gobyapi rtmilesapi rtaapi \
	msgsearchapi spxapi dfsapi ptop robais tsearchapi driviewapi \
	loghub topology crzptX crzptY ACB damS roadRank \
	gdgive bdgive gopath gomile ashman adcube adcube_v2\
	spark releaseServer dfsdb tsdb \
	timport msgimport rtimport simimport PMR dtsync pole-M pole-S tagpick trafficapi mttpServer mttpSvp rrtopo damR BRM mfptpServer pmrhttp weibo-S weibo-G
# club

help:
	@echo -e $(GREEN) "make libs first!" $(NONE)
	@echo -e $(RED) "make " $(BLUE) $(foreach n,$(SRV),[$(PURPLE)$(n)$(BLUE)]) $(NONE)
	@echo -e $(GREEN) "For more details, please look!!!" $(NONE)

libs:
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./lib
	#sleep 1
#	$(MAKE) -C ./engine distclean
#	$(MAKE) -j8 -C ./engine MAIN_SUPEX=supex_base
#	$(MAKE) -j8 -C ./engine MAIN_SUPEX=supex_scco
#	$(MAKE) -j8 -C ./engine MAIN_SUPEX=supex_line
#	$(MAKE) -j8 -C ./engine MAIN_SUPEX=supex_evuv
#	$(MAKE) -j8 -C ./engine MAIN_SUPEX=supex_evcoro
#	$(MAKE) -C ./engine xctl
#	$(MAKE) -C ./engine sclnt
	#sleep 1
	$(MAKE) -C ./open/lib clean
	$(MAKE) -C ./open/lib/
	cd lib/mapdata && make && cd $(HOME_PATH)


all:$(SRV)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)



releaseServer:#DATA_LOAD_TYPE=cfg or mysql
	$(MAKE) -C ./programs/releaseServer MAIN_APP_SERV=releaseServer DATA_LOAD_TYPE=cfg
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
driviewapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=driviewapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
ptop:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=ptop
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
spxapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=spxapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
dfsapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=dfsapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
rmsapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=rmsapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
hlsapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=hlsapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
robais:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=robais
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
tsearchapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=tsearchapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
msgsearchapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=msgsearchapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
weibo-S:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=weibo-S
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

drisamp:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=drisamp
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
drisampapi:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=drisampapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
drimode:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=drimode
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
drimodeapi:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=drimodeapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
goby:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=goby
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
gobyapi:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=gobyapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
rtmiles:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=rtmiles
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
rtmilesapi:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=rtmilesapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
rta:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=rta
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
rtaapi:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=rtaapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

topology:
	$(MAKE) -C ./programs/roadRelation MAIN_APP_SERV=topology
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

crzptX:
	$(MAKE) -C ./programs/crazyPoint MAIN_APP_SERV=crzptX
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

crzptY:
	$(MAKE) -C ./programs/crazyPoint MAIN_APP_SERV=crzptY
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

ACB:
	$(MAKE) -C ./programs/ACB MAIN_APP_SERV=ACB
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

damS:
	$(MAKE) -C ./programs/greatWall MAIN_APP_SERV=damS
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

damR:
	$(MAKE) -C ./programs/damR MAIN_APP_SERV=damR
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

loghub:
	$(MAKE) -C ./programs/loghub MAIN_APP_SERV=loghub
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

# dam:
# 	$(MAKE) -C ./programs/dam MAIN_APP_SERV=dam
# 	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

BRM:
	$(MAKE) -C ./programs/BRM MAIN_APP_SERV=BRM
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

roadRank:
	$(MAKE) -C ./programs/roadRank MAIN_APP_SERV=roadRank
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

dtsync:
	$(MAKE) -C ./programs/dtsync MAIN_APP_SERV=dtsync
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

# pole-M's compiler options.
#
# COMPILE: if not set with RELEASE, then it is DEBUG module.
# TCLEVEL: THREAD or COROUTINE,
#    If there isn't any clients, you can select THREAD,
#    other situation, you may select COROUTINE.
pole-M:
	$(MAKE) -C ./programs/pole-M MAIN_APP_SERV=pole-M COMPILE=RELEAS TCLEVEL=COROUTINE
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
pole-S:
	$(MAKE) -C ./programs/pole-S MAIN_APP_SERV=pole-S COMPILE=RELEAS
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

tagpick:
	$(MAKE) -C ./programs/tagpick MAIN_APP_SERV=tagpick
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gdgive:
	$(MAKE) -C ./programs/GIVE MAIN_APP_SERV=gdgive
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

bdgive:
	$(MAKE) -C ./programs/GIVE MAIN_APP_SERV=bdgive
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

mttpServer:
	$(MAKE) -C ./programs/mttpServer MAIN_APP_SERV=mttpServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

mttpSvp:
	$(MAKE) -C ./programs/mttpSvp MAIN_APP_SERV=mttpSvp
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


rrtopo:
	$(MAKE) -C ./programs/RRDemo/topo MAIN_APP_SERV=rrtopo
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gopath:
	$(MAKE) -C ./programs/flyKite MAIN_APP_SERV=gopath
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

gomile:
	$(MAKE) -C ./programs/flyKite MAIN_APP_SERV=gomile
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

kkb:
	$(MAKE) -C ./programs/flyKite MAIN_APP_SERV=kkb
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

ashman:
	$(MAKE) -C ./programs/ashman MAIN_APP_SERV=ashman
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

weibo-G:
	$(MAKE) -C ./programs/weibo-G MAIN_APP_SERV=weibo-G
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

useroptapi:
	$(MAKE) -C ./programs/useroptapi MAIN_APP_SERV=useroptapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


adcube_v2:
	$(MAKE) -C ./programs/adtalk_v2 MAIN_APP_SERV=adcube_v2
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
spark:
	$(MAKE) -C ./programs/adtalk_v2 MAIN_APP_SERV=spark
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

tsdb:
	$(MAKE) -C ./programs/TSDB MAIN_APP_SERV=tsdb
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

dfsdb:
	$(MAKE) -C ./programs/TSDB MAIN_APP_SERV=dfsdb
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

timport:
	$(MAKE) -C ./programs/timport MAIN_APP_SERV=timport
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

msgimport:
	$(MAKE) -C ./programs/timport MAIN_APP_SERV=msgimport
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

rtimport:
	$(MAKE) -C ./programs/timport MAIN_APP_SERV=rtimport
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

simimport:
	$(MAKE) -C ./programs/timport MAIN_APP_SERV=simimport
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

club:
	$(MAKE) -C ./programs/club MAIN_APP_SERV=club
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

trafficapi:
	$(MAKE) -C ./programs/trafficapi/ MAIN_APP_SERV=trafficapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

PMR:
	$(MAKE) -C ./programs/PMR MAIN_APP_SERV=PMR
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

pmrhttp:
	$(MAKE) -C ./programs/PMR_HTTP MAIN_APP_SERV=pmrhttp
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

mfptpServer:
	$(MAKE) -C ./programs/mfptpServer MAIN_APP_SERV=mfptpServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

coreExchangeNode:
	$(MAKE) -C ./programs/CoreExchangeNode MAIN_APP_SERV=coreExchangeNode
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

messageGateway:
	$(MAKE) -C ./programs/messageGateway MAIN_APP_SERV=messageGateway
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

push:
	git push origin HEAD:refs/for/devel

clean:
	$(MAKE) -C ./programs/damR clean
	$(MAKE) -C ./programs/RTAP clean
	$(MAKE) -C ./programs/MMR clean
	$(MAKE) -C ./programs/roadRelation clean
	$(MAKE) -C ./programs/crazyPoint clean
	$(MAKE) -C ./programs/ACB clean
	$(MAKE) -C ./programs/greatWall clean
	$(MAKE) -C ./programs/flyKite clean
	$(MAKE) -C ./programs/ashman clean
	$(MAKE) -C ./programs/weibo-G clean
	$(MAKE) -C ./programs/BRM clean
	$(MAKE) -C ./programs/roadRank clean
	$(MAKE) -C ./programs/dtsync clean
	$(MAKE) -C ./programs/pole-M clean
	$(MAKE) -C ./programs/pole-S clean
	$(MAKE) -C ./programs/tagpick clean
	$(MAKE) -C ./programs/GIVE clean
	$(MAKE) -C ./programs/mttpServer clean
	$(MAKE) -C ./programs/mttpSvp clean
	$(MAKE) -C ./programs/RRDemo/topo clean
	$(MAKE) -C ./programs/useroptapi clean
	$(MAKE) -C ./programs/releaseServer clean
	$(MAKE) -C ./programs/TSDB clean
	$(MAKE) -C ./programs/timport clean
	$(MAKE) -C ./programs/club clean
	$(MAKE) -C ./programs/trafficapi/ clean
	$(MAKE) -C ./programs/PMR clean
	$(MAKE) -C ./programs/PMR_HTTP clean
	$(MAKE) -C ./programs/loghub clean
	$(MAKE) -C ./programs/mfptpServer clean
	$(MAKE) -C ./programs/CoreExchangeNode clean

distclean:
	$(MAKE) -C ./programs/damR distclean
	$(MAKE) -C ./programs/RTAP distclean
	$(MAKE) -C ./programs/MMR distclean
	$(MAKE) -C ./programs/roadRelation distclean
	$(MAKE) -C ./programs/crazyPoint distclean
	$(MAKE) -C ./programs/ACB distclean
	$(MAKE) -C ./programs/greatWall distclean
	$(MAKE) -C ./programs/flyKite distclean
	$(MAKE) -C ./programs/ashman distclean
	$(MAKE) -C ./programs/weibo-G distclean
	$(MAKE) -C ./programs/BRM distclean
	$(MAKE) -C ./programs/roadRank distclean
	$(MAKE) -C ./programs/dtsync distclean
	$(MAKE) -C ./programs/pole-M distclean
	$(MAKE) -C ./programs/pole-S distclean
	$(MAKE) -C ./programs/tagpick distclean
	$(MAKE) -C ./programs/GIVE distclean
	$(MAKE) -C ./programs/mttpServer distclean
	$(MAKE) -C ./programs/mttpSvp distclean
	$(MAKE) -C ./programs/RRDemo/topo distclean
	$(MAKE) -C ./programs/useroptapi distclean
	$(MAKE) -C ./programs/loghub distclean
	$(MAKE) -C ./programs/releaseServer distclean
	$(MAKE) -C ./programs/TSDB distclean
	$(MAKE) -C ./programs/timport distclean
	$(MAKE) -C ./programs/trafficapi distclean
	$(MAKE) -C ./programs/PMR distclean
	$(MAKE) -C ./programs/PMR_HTTP distclean
	$(MAKE) -C ./programs/mfptpServer distclean
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./open/lib clean
	$(MAKE) -C ./engine distclean
	cd lib/mapdata && make clean && cd $(HOME_PATH)

########################################DONT USE#####################################################
install:
	cd lib/libev/  && ./configure && make && sudo make install
	sudo ldconfig
	@echo -e $(RED) "     >OK<" $(NONE)

other:
	yum install autoconf
	yum install automake.noarch
	yum install libtool.x86_64
	yum install libuuid-devel
	#yum install gcc-c++.x86_64
	#yum install libstdc++-devel.x86_64
	# mkdir /dev/mqueue
	# mount -t mqueue none /dev/mqueue

link:
	ln -s /usr/local/lib/libev.so  /usr/lib64/libev.so
	ln -s /usr/local/lib/libev.so.4  /usr/lib64/libev.so.4
	#链接是用绝对路径
	@echo "     >OK<"

tag:
	ctags -R

cs:
	cscope -Rbq

NAME = supex
VERSION = 2.0.0

tarball = $(NAME)-$(VERSION).tar.gz


$(tarball):
	git archive --format=tar.gz --prefix=$(NAME)-$(VERSION)/ HEAD > $@

