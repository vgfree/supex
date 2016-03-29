#!/bin/bash - 
#===============================================================================
#
#          FILE: cleanlog.sh
# 
#         USAGE: ./cleanlog.sh  [-t]  /pathname
# 
#   DESCRIPTION: 清理/pathname目录下所有修改日期为当天的日志
# 
#       OPTIONS: -t 可选参数，清理前备份日志文件
			
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: chenxijun@mirrtalk.com
#  ORGANIZATION: 
#       CREATED: 2015年12月01日 14:41
#      REVISION:  ---
#===============================================================================
DATE=$(date +%Y%m%d%H%M)


help () {
	local command=`basename $0`
	echo "${basename} -t /pathname"
}

args=$#
pathname="${!args}"
echo ${pathname}|grep -E '^\/' >/dev/null 2>&1 ||  stat='please input filepath!'
if [ "${stat}" = 'please input filepath!' ];then
   	echo "${pathname} is not a full pathname" 1>&2
	exit 1
else
	pathname="${!args}"
        arr_file=($(find ${pathname} -mtime +1  -name "*.log"))                             
#       echo ${arr_file[@]} 

fi
if [  ${#arr_file[@]} -eq 0 ];then
#	echo "no log file find";
	exit 1;
fi

baklog () {
	for file in ${arr_file[@]};
	do
		path=${file%/*}
		file_name=${file##*/}
		tar -zcvf ${file}_${DATE}.tar.gz -C ${path} ${file_name} >/dev/null 2>&1
	done
}

cleanlog () {
	for file in ${arr_file[@]};
	do
		if [ -f ${file} -a ${file##*.} = "log"  ];then
			echo "rm ${file}"
			rm ${file};
		fi
	done
}


#input
while getopts t opt
do
        case "$opt" in
	
        t) 

			baklog
		;;

	*) echo "Invalid option: -$OPTARG"
		exit 2
		;;
        esac
done

cleanlog

exit 0
