#!/usr/bin/expect

spawn ssh 192.168.1.188 
expect "172.16"
send "touch test\n"
#spawn ssh  mtdk@172.16.21.178
#expect "21.101"
#send "touch test\n"
expect timeout
exit
