#!/bin/bash

#######################################
##选择更新类型：半更新or全更新,定义域名
#######################################
Read()
{
		read -p "please enter your domain:" Domain
		read -p "enter the file's absolute path:" File
		Ip
}

############################定义服务器的IP
Ip()
{
		read -p "Please enter your deployment of IP:" IP 
		Ipcheck
}

#############################判断IP是否正确
Ipcheck()
{
if [[ "$IP" =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]];then
        for i in {1..4}
        do
                NUM=$(echo $IP |cut -d. -f$i)
                [ $NUM -gt 254 ] && echo "The ip is wrong" && Ip
        done
else
        echo "The ip is wrong" && Ip
fi
	Check
}
############################1.188 确认代码是否有更新
Check()
{
DIR=/data/idc/ngxapi_v2

	if [ -d $DIR/$Domain ]
then
        cd $DIR/$Domain/api | git pull
		for i in $File
	do
			if [ -f $i ]
		then
			md5sum $i && Upload
		else
			echo "No such file"  && exit 1
		fi
	done
else
	echo "No such directory" && exit 2
fi


}


###############################上传代码到入口机/tmp下
Upload()
{
	scp -P 16861 $i 172.16.21.101:/tmp 
	if [ $? -eq 0 ]
then
	Server
else
	echo "upload failed" && exit 3
	fi
}

################################上传代码到现网服务器
Server()
{
	expect /data/scripts/lua.ex $IP $Packet $Domain  $MD51 $i


}

Read

