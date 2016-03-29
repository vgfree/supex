RUN_COUNT=`ps -elf | grep "sh ${0}" | grep -v "grep" | wc -l`
echo ${RUN_COUNT}
if [ ${RUN_COUNT} -gt 2 ];then
	echo "$0 has alreay run, please try latter!"
	exit 1
fi

if [ $# -ne 2 ];then
	echo "usage: \"sh supex_make_tags.sh BRANCH TAGS\""
	exit 1
fi

BRANCH=$1
PROJECT=$2
NEWTREE=supex
GITUSER=tianlu
TAGNAME=$PROJECT-`date "+%G-%m-%d_%H-%M-%S"`

git clone ssh://$GITUSER@192.168.71.33:29418/supex.git $NEWTREE
if [ $? -ne 0 ];then
	echo "git clone $NEWTREE error"
	exit 1
fi

cd $NEWTREE 
if [ $BRANCH != "master" ]; then
	git checkout -b $BRANCH origin/$BRANCH
	if [ $? -ne 0 ];then	
		echo "git checkout branch($BRANCH) of supex error"
		exit 1
	fi
fi
echo "the branch is $BRANCH"
sleep 1

echo "make ${PROJECT}"
sleep 1
sed -i -e "s/EXPORT_CFLAGS += -DUSE_MEMHOOK/#EXPORT_CFLAGS += -DUSE_MEMHOOK/g" Makefile
make libs && make clean && make ${PROJECT} && cd ..

if [ ! -d $NEWTREE ];then
	exit 1
fi

mkdir $TAGNAME

if [ ${PROJECT} == "goby" ];then
	RUN_CFLAGS="-t load_topology"
fi

MAKEFILE="\nall:\n\
	\techo \"make run\"\n\
	run:\n\
	\t@-if [ ! -d ./logs ];then mkdir logs; fi\n\
	\tnohup ./$PROJECT ${RUN_CFLAGS} &"

APPEND="load:\n\
	\t@-if [ ! -d ./logs ];then mkdir logs; fi\n\
	\t@sh init/init.sh $PROJECT\n\
	\t./$PROJECT &\n\
	\tsleep 3\n\
	\tluajit init/$PROJECT""_load.lua\n\
	\tkillall $PROJECT\n"



function copy_open(){
	mkdir $TAGNAME/open
	cp -r $NEWTREE/open/apply/*.lua				$TAGNAME/open
	cp -r $NEWTREE/open/linkup/*.lua			$TAGNAME/open
	cp -r $NEWTREE/open/public/*.lua			$TAGNAME/open
	cp -r $NEWTREE/open/spxonly/*.lua			$TAGNAME/open
	cp -r $NEWTREE/open/lib/*.lua				$TAGNAME/open
	cp -r $NEWTREE/open/lib/*.so				$TAGNAME/open
	cp -r $NEWTREE/open/lib/resty				$TAGNAME/open
	cp -r $NEWTREE/open/lib/lua-http-message		$TAGNAME/open
	cp $NEWTREE/engine/sclnt $TAGNAME
}


case $PROJECT in
	"crzptY")
		copy_open

		cp -r $NEWTREE/programs/crazyPoint/crzpt_lua		$TAGNAME
		mkdir $TAGNAME/swift_lua
		cp -r $NEWTREE/programs/crazyPoint/swift_lua/core	$TAGNAME/swift_lua
		cp -r $NEWTREE/programs/crazyPoint/swift_lua/$PROJECT	$TAGNAME/swift_lua
		cp -r $NEWTREE/programs/crazyPoint/cfg.lua		$TAGNAME
		cp -r $NEWTREE/programs/crazyPoint/$PROJECT		$TAGNAME
		cp -r $NEWTREE/programs/crazyPoint/$PROJECT"_conf.json" $TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"gopath")
		copy_open

		mkdir $TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/flyKite/sniff_lua/core			$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/flyKite/sniff_lua/$PROJECT		$TAGNAME/sniff_lua
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/cfg.lua			$TAGNAME
		cp -r $NEWTREE/programs/flyKite/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/flyKite/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"gomile")
		copy_open

		mkdir $TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/flyKite/sniff_lua/core			$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/flyKite/sniff_lua/$PROJECT		$TAGNAME/sniff_lua
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/cfg.lua			$TAGNAME
		cp -r $NEWTREE/programs/flyKite/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/flyKite/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"ashman")
		copy_open

		mkdir $TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/ashman/sniff_lua/core			$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/ashman/sniff_lua/$PROJECT		$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/ashman/cfg.lua				$TAGNAME
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/ashman/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/ashman/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"damS")
		cp -r $NEWTREE/programs/greatWall/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/greatWall/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"goby")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/$PROJECT			$TAGNAME/lua
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/cfg.lua			$TAGNAME
		mkdir $TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/init.sh			$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/$PROJECT"_load.lua"		$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/MMR/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		echo -e $APPEND >> $TAGNAME/Makefile
		;;
	"checkequipment")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/checkequipment/lua/core			$TAGNAME/lua
		cp -r $NEWTREE/programs/checkequipment/lua/$PROJECT			$TAGNAME/lua
		cp -r $NEWTREE/programs/checkequipment/cfg.lua				$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/checkequipment/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/checkequipment/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"drimode")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/$PROJECT			$TAGNAME/lua
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		mkdir $TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/init.sh			$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/$PROJECT"_load.lua"		$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/MMR/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		echo -e $APPEND >> $TAGNAME/Makefile
		;;
	"topology")
		cp -r $NEWTREE/programs/roadRelation/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/roadRelation/$PROJECT"_conf.json"	$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"weibo")
		cp -r $NEWTREE/programs/weibo/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/weibo/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"weidb")
		copy_open

		mkdir $TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/weidb/sniff_lua/core			$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/weidb/sniff_lua/$PROJECT		$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/weidb/cfg.lua				$TAGNAME
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/weidb/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/weidb/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"ptop")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/$PROJECT			$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/cfg.lua				$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"spxapi")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/$PROJECT			$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/cfg.lua				$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"drisamp")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/$PROJECT			$TAGNAME/lua
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/cfg.lua			$TAGNAME
		mkdir $TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/init.sh			$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/$PROJECT"_load.lua"		$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/MMR/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		echo -e $APPEND >> $TAGNAME/Makefile
		;;
	"rtmiles")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/MMR/lua/$PROJECT			$TAGNAME/lua
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/cfg.lua			$TAGNAME
		mkdir $TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/init.sh			$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/init/$PROJECT"_load.lua"		$TAGNAME/init
		cp -r $NEWTREE/programs/MMR/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/MMR/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		echo -e $APPEND >> $TAGNAME/Makefile
		;;
	"ACB")
		copy_open

		mkdir $TAGNAME/swift_lua
		cp -r $NEWTREE/programs/ACB/swift_lua/core	$TAGNAME/swift_lua
		cp -r $NEWTREE/programs/ACB/swift_lua/$PROJECT	$TAGNAME/swift_lua
		cp -r $NEWTREE/programs/ACB/cfg.lua		$TAGNAME
		mv $TAGNAME/swift_lua/$PROJECT/deploy/link.lua	$TAGNAME
		cp -r $NEWTREE/programs/ACB/$PROJECT		$TAGNAME
		cp -r $NEWTREE/programs/ACB/$PROJECT"_conf.json" $TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"gdgive")
		cp -r $NEWTREE/programs/GIVE/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/GIVE/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"bdgive")
		cp -r $NEWTREE/programs/GIVE/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/GIVE/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"mttpServer")
		cp -r $NEWTREE/programs/mttpServer/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/mttpServer/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"tsdb")
		cp -r $NEWTREE/programs/TSDB/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/TSDB/$PROJECT"_conf.json"		$TAGNAME
		mkdir -p $TAGNAME/var/data $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"timport")
		cp -r $NEWTREE/programs/timport/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/timport/$PROJECT"_conf.json"		$TAGNAME
		mkdir -p $TAGNAME/var/backup $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"msgimport")
		cp -r $NEWTREE/programs/timport/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/timport/$PROJECT"_conf.json"		$TAGNAME
		mkdir -p $TAGNAME/var/backup $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"rtimport")
		cp -r $NEWTREE/programs/timport/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/timport/$PROJECT"_conf.json"		$TAGNAME
		mkdir -p $TAGNAME/var/backup $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"simimport")
		cp -r $NEWTREE/programs/timport/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/timport/$PROJECT"_conf.json"		$TAGNAME
		mkdir -p $TAGNAME/var/backup $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"dfsdb")
		cp -r $NEWTREE/programs/TSDB/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/TSDB/$PROJECT"_conf.json"		$TAGNAME
	
mkdir -p $TAGNAME/var/data $TAGNAME/var/logs
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"adcube_v2")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/adtalk_v2/lua/core		$TAGNAME/lua
		cp -r $NEWTREE/programs/adtalk_v2/lua/adcube_v2		$TAGNAME/lua
		mv $TAGNAME/lua/adcube_v2/deploy/link.lua			$TAGNAME
		cp  $NEWTREE/programs/adtalk_v2/$PROJECT"_conf.json" 	$TAGNAME
		cp  $NEWTREE/programs/adtalk_v2/"cfg.lua"		$TAGNAME
		cp  $NEWTREE/programs/adtalk_v2/$PROJECT		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile

		;;
	"BRM")
		cp -r $NEWTREE/programs/BRM/$PROJECT			$TAGNAME
		cp -r $NEWTREE/programs/BRM/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"dfsapi")
		copy_open
		cp -r $NEWTREE/open/lib/magick					$TAGNAME/open
		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/core				$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/$PROJECT			$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/cfg.lua				$TAGNAME
		mv $TAGNAME/lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT				$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"weibo-G")
		copy_open

		mkdir $TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/weibo-G/sniff_lua/core		$TAGNAME/sniff_lua
		cp -r $NEWTREE/programs/weibo-G/sniff_lua/weibo-G	$TAGNAME/sniff_lua
		cp -r $NEWTREE/open/lib/lua-coro/ring.lua		$TAGNAME/sniff_lua/core
		cp -r $NEWTREE/open/lib/lua-coro/coro.lua		$TAGNAME
		mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
		cp  $NEWTREE/programs/weibo-G/$PROJECT"_conf.json" 	$TAGNAME
		cp  $NEWTREE/programs/weibo-G/sniff_lua/weibo-G/deploy/cfg.lua		$TAGNAME
		cp  $NEWTREE/programs/weibo-G/$PROJECT			$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	"weibo-S")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/core 			$TAGNAME/lua
		cp -r $NEWTREE/programs/RTAP/lua/weibo-S		$TAGNAME/lua
		cp -r $NEWTREE/open/lib/lua-coro/ring.lua		$TAGNAME/lua/core
		cp -r $NEWTREE/open/lib/lua-coro/coro.lua		$TAGNAME/open
		mv $TAGNAME/lua/weibo-S/deploy/link.lua			$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT"_conf.json" 	$TAGNAME
		cp -r $NEWTREE/programs/RTAP/cfg.lua			$TAGNAME
		cp -r $NEWTREE/programs/RTAP/$PROJECT			$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	*)
		echo -e "\n"$COLOR_RED"NO PROJECT NAME!"$COLOR_DEFAULT;;
esac

#rm -rf $NEWTREE
if [ $# -eq 3 ];then
	OLD_TIME=${2#*-}
	OLD_TIME=${OLD_TIME%/*}
	NEW_TIME=${TAGNAME#*-}
	diff -upNr $2 $TAGNAME > $PROJECT"___"$OLD_TIME"__"$NEW_TIME".patch"
fi
tar -jcvf $TAGNAME".tar.bz2" $TAGNAME
#rm -rf $TAGNAME
