#!/usr/bin/expect

set IP [lindex $argv 0]
set Packet [lindex $argv 1] 
set Domain [lindex $argv 2]
set MD51 [lindex $argv 3]
set File [lindex $argv 4]

spawn ssh -p 16861 172.16.21.101
expect {
	"yes/no" {send "yes\n";exp_continue}
	"from" {send "scp -P 16861  /tmp/$Packet $IP:/tmp\n"}
}
expect "$"
send "scp -P 16861 /tmp/lua2.sh $IP:/tmp\n"
expect "#"
send "ssh  mtdk@$IP\n"
expect {
	"yes/no" {send "yes\n";exp_continue}
	"from" {send "bash -x -v /tmp/lua2.sh $Domain $Packet $File\n"}
}
expect eof
exit


