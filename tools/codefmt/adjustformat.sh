##############################################################################################
#____________________________________________________________________________________________#
#Reference:
#This shell script can adjust  whitespace formatting of file, but only the file can be parse
#by uncrustity, such file type like '*.c', '*.cpp', '*.lua', '*.sh' etc.
#if you want running this script you need install uncrustity in your computer first!

#____________________________________________________________________________________________#
#How is this shell script works:
#copy 'defaultstyle.cfg' file and 'adjustformat.sh' in same directory, and running this
#script as bellow:[adjustformat.sh file  must be executable]

#./adjustformat.sh  filename

#'filename' can be file or directory which you want adjust its formatting, if it's directory,
#this script will adjust every file in this directory and every file in its subdirectory that 
#uncrustity can parse 

#if you don't like the  formatting style of 'defaultstyle.cfg' file , you can change value 
#of parameter in 'defaultstyle.cfg' file and make it to your style

#____________________________________________________________________________________________#
#Reference of variables:UnchangeFile & UnchangeDir & LogFile

#you can change contents of UnchangeFile and UnchangeDir to your demand,that means you can add 
#files which you don't want change their formatting or delete them when you change your mind.

#LogFile just in case of you want to know details about this script modified which files, if
#you are interested about its details you can change LogFile to your own path of log file, and 
#cancel comments in front of each 'echo' in bellow. if you don't really care about details just
#ignore this variable

#____________________________________________________________________________________________#
#Notes: 
#you can't count on uncrustity won't go any wrong,here is uncrustity's README says:
#"As of the current release, I don't particularly trust this program 
# to not make mistakes and screw up my whitespace formatting."

#if you don't really care about the odds of uncrustity may make mistakes,then just ignore what 
#I am about to say,if you are not sure about the result of uncrustity's operation and don't want 
#uncrustity srcew up your files formatting a little bit by  any chance,in that case,you can do as
#bellow:

#comment 'mv -f ${file}.unc $file',that means put a '#' in front of line 78 

#you don't need worry about anything if you comment line 78, because what uncrustity modified 
#all in '${file}.unc' file not in your source file, you can use 'vimdiff' command checkout the 
#difference between those two files, if uncrustify didn't do what you want ,you can change the 
#value of parameter in 'defaultstyle.cfg' file until it does what you want,and then you may cancel 
#the comment in front of line 78

#But if you comment line 78, you will have many '*.unc' files in your folders which have files 
#need to be adjust formatting,so choice wisely, whatever,it's your call !

#____________________________________________________________________________________________#

##############################################################################################

#!/bin/bash

HomeDir=`pwd`
Objective=$1
LogFile=/home/xuli/Desktop/log  #change to your own path of log file or just ignore it
CfgFile=defaultstyle.cfg #the default style for your adjustion formatting

declare -a UnchangeFile UnchangeDir
UnchangeFile=($CfgFile adjustformat.sh)
UnchangeDir=(cJSON http-parser libzdb hiredis graphlabapi json-c FLIF sqlite-autoconf-3090200 leveldb-1.19  libev libkv zeromq-4.2.1 luasocket lua-cjson-2.1.0 Lua-bit32 lua-zlib libconhash luafilesystem luasql-master)


StartAdjust()
{
	local file=$1
	local temp=${file##*.}

	if [ $temp == "c" -o $temp == "h" ];then
		uncrustify -c $HomeDir/$CfgFile -l C -f $file > ${file}.unc
		if [ $? == 0 ];then
			mv -f ${file}.unc $file
		else
			echo -e "\x1B[1;31m"$file" ERROR\n"
			exit 0
		fi
		#	echo  "adjust file: $file"  >> $LogFile
	elif [ $temp == "cpp" -o $temp == "hpp" -o $temp == "cc" ];then
		uncrustify -c $HomeDir/$CfgFile -l CPP -f $file > ${file}.unc 
		if [ $? == 0 ];then
			mv -f ${file}.unc $file
		else
			echo -e "\x1B[1;31m"$file" ERROR\n"
			exit 0
		fi
	elif [ $temp == "lua" ];then
		#lua lunadry.lua -i $file
		if [ $? == 0 ];then
			echo -e "\x1B[1;32m"$file" Done\n"
		else
			echo -e "\x1B[1;31m"$file" ERROR\n"
			exit 0
		fi
	fi
}

Comparefile()
{
	#echo ---------------------------------------------- >> $LogFile

	local flag=0
	local path=$1
	local file=${path##*/}

	#echo "compare filename:$path" >> $LogFile

	for i in ${UnchangeFile[@]}
	do
		if [ $i == $file ];then
			#		echo "comparefile matched: $file,skip this one and process next one" >> $LogFile
			flag=$[$flag + 1]
			break
		else
			continue
		fi
	done

	if [ $flag -eq 0 ];then
		#	echo "comparefile didn't match: $file, processing...." >> $LogFile
		StartAdjust "$path"
	fi

	#echo ---------------------------------------------- >> $LogFile
}

Comparedir()
{
	#echo ---------------------------------------------- >> $LogFile

	local path=$1
	local dir=${path##*/}
	local flag=0

	#echo "comparedirname:$path" >> $LogFile

	for i in ${UnchangeDir[@]}
	do
		if [ $i == $dir ];then
			#		echo "comparedir matched: $dir, skip this one and process next one" >> $LogFile
			flag=$[$flag+1]
			break
		else
			continue
		fi
	done

	if [ $flag -eq 0 ];then
		#	echo "comparedir didn't match: $dir, processing...." >> $LogFile
		Loop "$path"
	fi

	#echo ---------------------------------------------- >> $LogFile

}

Loop()
{
	#echo ------------------------------------------ >> $LogFile
	#echo --------- "Start Loop" ------------------- >> $LogFile

	local path=$1
	local filelist=`ls $path`

	#echo "path:$path" >> $LogFile
	#echo "filelist: $filelist" >> $LogFile

	for i in $filelist
	do
		if [ -f "$path/$i" ];then
			Comparefile "$path/$i"
		elif [ -d "$path/$i" ];then
			Comparedir "$path/$i"
		else
			continue
		fi
	done

	#echo ----------- "End Loop" ------------------- >> $LogFile
	#echo ------------------------------------------ >> $LogFile
}


if [ -d $Objective ];then

	if [ ${Objective:0-1:1} == '/' ];then
		Loop  "${Objective%/*}"
	else
		Loop  "$Objective"
	fi

elif [ -f $Objective ];then
	Comparefile "$Objective"
else
	echo "The file type is wrong,can't parse it"
fi
