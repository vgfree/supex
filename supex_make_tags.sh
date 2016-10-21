#make sure one run in the same time.
RUN_COUNT=`ps -elf | grep "sh ${0}" | grep -v "grep" | wc -l`
echo ${RUN_COUNT}
if [ ${RUN_COUNT} -gt 2 ];then
	echo "$0 has alreay run, please try latter!"
	exit 1
fi



function clone()
{
	if [ $# -ne 1 ];then
		echo "usage: \"sh supex_make_tags.sh -c GITUSER\""
		exit 1
	fi
	GITUSER=$1
	NEWTREE=supex
	git clone ssh://$GITUSER@192.168.71.33:29418/$NEWTREE.git
	if [ $? -ne 0 ];then
		echo "git clone $NEWTREE error"
		exit 1
	fi
	cd $NEWTREE 
}

function branch()
{
	if [ $# -ne 1 ];then
		echo "usage: \"sh supex_make_tags.sh -b BRANCH\""
		exit 1
	fi
	BRANCH=$1
	if [ $BRANCH != "master" ]; then
		git checkout -b $BRANCH origin/$BRANCH
		if [ $? -ne 0 ];then	
			echo "git checkout branch($BRANCH) of supex error"
			exit 1
		fi
	fi
	echo "the branch is $BRANCH"
}





function openlibs()
{
	sed -i -e "s/^EXPORT_CFLAGS += -DUSE_MEMHOOK/#EXPORT_CFLAGS += -DUSE_MEMHOOK/g" Makefile
	make libs
}




function makeproj()
{
	if [ $# -ne 1 ];then
		echo "usage: \"sh supex_make_tags.sh -p PROJECT\""
		exit 1
	fi
	PROJECT=$1
	make clean
	echo "make ${PROJECT}"
	make ${PROJECT}
}

function _copy_open_()
{
	mkdir $2/open
	cp -r $1/open/apply/*.lua			$2/open
	cp -r $1/open/linkup/*.lua			$2/open
	cp -r $1/open/public/*.lua			$2/open
	cp -r $1/open/spxonly/*.lua			$2/open
	cp -r $1/open/lib/*.lua				$2/open
	cp -r $1/open/lib/*.so				$2/open
	cp -r $1/open/lib/resty				$2/open
	cp -r $1/open/lib/lua-http-message		$2/open
}


function maketags()
{
	if [ $# -ne 1 ];then
		echo "usage: \"sh supex_make_tags.sh -p PROJECT\""
		exit 1
	fi
	PROJECT=$1
	TAGNAME=$PROJECT-`date "+%G-%m-%d_%H-%M-%S"`
	mkdir $TAGNAME
	if [ ! -d $TAGNAME ];then
		exit 1
	fi


	MAKEFILE="\nall:\n\
		\techo \"make run\"\n\
		run:\n\
		\t@-if [ ! -d ./logs ];then mkdir logs; fi\n\
		\tnohup ./$PROJECT &"

	APPEND="load:\n\
		\t@-if [ ! -d ./logs ];then mkdir logs; fi\n\
		\t@sh init/init.sh $PROJECT\n\
		\t./$PROJECT &\n\
		\tsleep 3\n\
		\tluajit init/$PROJECT""_load.lua\n\
		\tkillall $PROJECT\n"


		NEWTREE="./"
		case $PROJECT in
		"loghub")
			_copy_open_ $NEWTREE $TAGNAME

			mkdir $TAGNAME/lua
			cp -r $NEWTREE/lua/*				$TAGNAME/lua
			cp -r $NEWTREE/cfg.lua				$TAGNAME
			cp -r $NEWTREE/link.lua				$TAGNAME
			cp -r $NEWTREE/$PROJECT				$TAGNAME
			cp -r $NEWTREE/$PROJECT"_conf.json"		$TAGNAME
			echo -e $MAKEFILE > $TAGNAME/Makefile
			;;
		"loghit")
			cp -r $NEWTREE/$PROJECT				$TAGNAME
			cp -r $NEWTREE/$PROJECT"_conf.json"		$TAGNAME
			echo -e $MAKEFILE > $TAGNAME/Makefile
			;;
		"crzptY")
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
		
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
			_copy_open_ $NEWTREE $TAGNAME
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
			_copy_open_ $NEWTREE $TAGNAME
		
			mkdir $TAGNAME/sniff_lua
			cp -r $NEWTREE/programs/weibo-G/sniff_lua/core		$TAGNAME/sniff_lua
			cp -r $NEWTREE/programs/weibo-G/sniff_lua/weibo-G	$TAGNAME/sniff_lua
			cp -r $NEWTREE/open/lib/lua-coro/ring.lua		$TAGNAME/sniff_lua/core
			cp -r $NEWTREE/open/lib/lua-coro/coro.lua		$TAGNAME/open
			mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua		$TAGNAME
			cp  $NEWTREE/programs/weibo-G/$PROJECT"_conf.json" 	$TAGNAME
			cp  $NEWTREE/programs/weibo-G/sniff_lua/weibo-G/deploy/cfg.lua		$TAGNAME
			cp  $NEWTREE/programs/weibo-G/$PROJECT			$TAGNAME
			echo -e $MAKEFILE > $TAGNAME/Makefile
			;;
		"weibo-S")
			_copy_open_ $NEWTREE $TAGNAME
		
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
		"mttpSvp")
			_copy_open_ $NEWTREE $TAGNAME
		
			mkdir $TAGNAME/sniff_lua
			cp -r $NEWTREE/programs/mttpSvp/sniff_lua/core			$TAGNAME/sniff_lua
			cp -r $NEWTREE/programs/mttpSvp/sniff_lua/mttpSvp		$TAGNAME/sniff_lua
			mv $TAGNAME/sniff_lua/$PROJECT/deploy/link.lua			$TAGNAME
			cp  $NEWTREE/programs/mttpSvp/$PROJECT"_conf.json"		$TAGNAME
			cp  $NEWTREE/programs/mttpSvp/sniff_lua/mttpSvp/deploy/cfg.lua	$TAGNAME
			cp  $NEWTREE/programs/mttpSvp/$PROJECT				$TAGNAME
			echo -e $MAKEFILE > $TAGNAME/Makefile
			;;
		
		*)
			echo -e "\n"$COLOR_RED"NO PROJECT NAME!"$COLOR_DEFAULT;;
	esac
	tar -jcvf $TAGNAME".tar.bz2" $TAGNAME
}


function diffpatch()
{
	if [ $# -eq 3 ];then
		OLD_TIME=${2#*-}
		OLD_TIME=${OLD_TIME%/*}
		NEW_TIME=${1#*-}
		PROJECT=$3
		diff -upNr $2 $1 > $PROJECT"___"$OLD_TIME"__"$NEW_TIME".patch"
	fi
}



case $1 in
	"-c")
		clone $2
		;;
	"-b")
		branch $2
		;;
	"-l")
		openlibs
		;;
	"-p")
		makeproj $2
		maketags $2
		;;
	"-d")
		diffpatch $2 $3 $4
		;;
	*)
		echo -e "\n"$COLOR_RED"\t-c[GITUSER]\n\t-b[BRANCH]\n\t-l\n\t-p[PROJECT]\n\t-d[NEWTAG][OLDTAG][PROJECT]\n"$COLOR_DEFAULT;;
esac

