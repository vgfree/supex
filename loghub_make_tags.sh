

PROJECT=$1
NEWTREE=loghub
GITUSER=mengxd
TAGNAME=$PROJECT-`date "+%G-%m-%d_%H-%M-%S"`

if [ $# -eq 0 ];then
	echo -e "\n"$COLOR_RED"NO PROJECT NAME!"$COLOR_DEFAULT
	exit
fi
git clone ssh://$GITUSER@192.168.1.5:29418/${NEWTREE} $NEWTREE
cd $NEWTREE && make clean && make libs && make ${PROJECT} && cd ..

if [ ! -d $NEWTREE ];then
	exit
fi

mkdir $TAGNAME

MAKEFILE="\nall:\n\
\techo \"make run\"\n\
run:\n\
\t@-if [ ! -d ./logs ];then mkdir logs; fi\n\
\tnohup ./$PROJECT &"



function copy_open(){
	mkdir $TAGNAME/open
	cp -r $NEWTREE/open/apply/*.lua				$TAGNAME/open
	cp -r $NEWTREE/open/linkup/*.lua			$TAGNAME/open
	cp -r $NEWTREE/open/public/*.lua			$TAGNAME/open
	cp -r $NEWTREE/open/lib/*.lua				$TAGNAME/open
	cp -r $NEWTREE/open/lib/*.so				$TAGNAME/open
	cp -r $NEWTREE/open/lib/resty				$TAGNAME/open
}


case $PROJECT in
	"loghub")
		copy_open

		mkdir $TAGNAME/lua
		cp -r $NEWTREE/lua/*				$TAGNAME/lua
		cp -r $NEWTREE/cfg.lua				$TAGNAME
		cp -r $NEWTREE/link.lua				$TAGNAME
		cp -r $NEWTREE/$PROJECT				$TAGNAME
		cp -r $NEWTREE/$PROJECT"_conf.json"		$TAGNAME
		echo -e $MAKEFILE > $TAGNAME/Makefile
		;;
	*)
		echo -e "\n"$COLOR_RED"NO PROJECT NAME!"$COLOR_DEFAULT;;
esac


#rm -rf $NEWTREE
if [ $# -eq 2 ];then
	OLD_TIME=${2#*-}
	OLD_TIME=${OLD_TIME%/*}
	NEW_TIME=${TAGNAME#*-}
	diff -upNr $2 $TAGNAME > $PROJECT"___"$OLD_TIME"__"$NEW_TIME".patch"
fi
tar -jcvf $TAGNAME".tar.bz2" $TAGNAME
rm -rf $TAGNAME
